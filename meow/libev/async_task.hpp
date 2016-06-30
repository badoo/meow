////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <thread>
#include <mutex>
#include <atomic>
#include <utility>

#include <boost/noncopyable.hpp>

#include <meow/std_unique_ptr.hpp>
#include <meow/ptr_list.hpp>
#include <meow/unique_id.hpp>
#include <meow/utility/offsetof.hpp>


////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct async_task_t;
	struct async_executor_t;

	typedef std::unique_ptr<async_task_t> async_task_ptr;
	typedef std::unique_ptr<async_executor_t> async_executor_ptr;

	// stored by executor, given to all tasks when they run
	//  useful for storing state that is attached to event loop (like idle runqueue)
	struct async_env_t : private boost::noncopyable
	{
		~async_env_t() {}
	};
	typedef std::unique_ptr<async_env_t> async_env_ptr;

	struct async_executor_t : private boost::noncopyable
	{
		virtual ~async_executor_t() {}

		void startup() { this->startup([](evloop_t*) { return meow::make_unique<async_env_t>(); }); }

		virtual void startup(std::function<async_env_ptr(evloop_t*)> const& env_init) = 0;

		virtual async_env_t* task_env() const = 0;      // environment for all tasks

		virtual void run_task(async_task_ptr) = 0;      // main thread only
		virtual void finalize_task(async_task_t*) = 0;  // worker thread only
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct async_task_t : public meow::ptr_list_hook_t
	{
		uint64_t const   unique_id;

	public:

		async_task_t()
			: unique_id(meow::unique_id<struct tag>::generate_unique_id())
		{
		}

		virtual ~async_task_t() {}

		evloop_t*    loop() const { return loop_; }
		async_env_t* env()  const { return executor_->task_env(); }

	public:

		void task_run(async_executor_t *e, evloop_t *loop) // worker thread
		{
			loop_ = loop;
			executor_ = e;
			this->do_task_run();
		}

		void task_abort() // worker thread
		{
			this->do_task_abort();
		}

		void task_finished(evloop_t *loop) // main thread
		{
			loop_ = loop;
			this->do_finished();
		}

	protected:

		void task_finalize() // worker thread
		{
			executor_->finalize_task(this);
		}

	protected:

		virtual void do_task_run() = 0;
		virtual void do_task_abort() = 0;
		virtual void do_finished() = 0;

	private:
		evloop_t         *loop_;
		async_executor_t *executor_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct async_executor___threaded_t : public async_executor_t
	{
		using self_t  = async_executor___threaded_t;
		using queue_t = meow::ptr_list_t<async_task_t>;

		virtual async_env_t* task_env() const override
		{
			return thr_ctx_->task_env.get();
		}

		virtual void run_task(async_task_ptr task) override
		{
			{
				std::lock_guard<std::mutex> g_(thr_ctx_->incoming_mtx);
				if (this->shutting_down != 0)
					return;

				thr_ctx_->incoming_q.push_back(move(task));
			}

			this->thread_notify();
		}

		virtual void finalize_task(async_task_t *task_p) override
		{
			async_task_ptr task = thr_ctx_->running_q.grab_by_ptr(task_p);

			{
				std::lock_guard<std::mutex> g_(this->results_mtx);
				this->results_q.push_back(move(task));
			}

			this->executor_notify();
		}

	public:

		async_executor___threaded_t(evloop_t *loop)
			: loop_(loop)
			, shutting_down(0)
		{
			ev_async_init(&ev_, &executor_wakeup);
			ev_async_start(loop_, &ev_);
		}

		~async_executor___threaded_t()
		{
			if (thr_ctx_)
			{
				{
					std::lock_guard<std::mutex> lk_(thr_ctx_->incoming_mtx);
					this->shutting_down = true;
				}

				this->thread_notify();
				thr_->join();
			}

			ev_async_stop(loop_, &ev_);
		}

		virtual void startup(std::function<async_env_ptr(evloop_t*)> const& task_env_init_func) override
		{
			// this happens in main thread for a reason
			//  we want to have everything fully initialized before thread starts (at unspecified point in the future)
			thr_ctx_ = [&]()
			{
				auto ctx = meow::make_unique<thread_ctx_t>();
				ctx->loop = libev::create_loop(EVFLAG_AUTO | EVFLAG_NOENV);

				ctx->ev.data = this;
				ev_async_init(&ctx->ev, &thr_wakeup);
				ev_async_start(ctx->loop.get(), &ctx->ev);

				return move(ctx);
			}();

			// lambda initializers not supported in C++11 yet, so do it explicitly
			thread_ctx_t *ctx = thr_ctx_.get();

			std::atomic_thread_fence(std::memory_order_seq_cst); // make sure everything is constructed before thread starts

			thr_ = meow::make_unique<std::thread>([ctx, task_env_init_func]
			{
				ctx->task_env = task_env_init_func(ctx->loop.get());

				std::atomic_thread_fence(std::memory_order_seq_cst); // make sure ctx->task_env write is visible to event loop

				libev::run_loop(ctx->loop);
			});
		}

	private:

		static void thr_wakeup(evloop_t *loop, evasync_t *ev, int revents)
		{
			auto *executor = static_cast<self_t*>(ev->data);
			auto *ctx = MEOW_SELF_FROM_MEMBER(thread_ctx_t, ev, ev);

			bool shutting_down;
			queue_t local_q;
			{ // grab everything from incoming queue to local queue to reduce locking
				std::lock_guard<std::mutex> lk_(ctx->incoming_mtx);
				shutting_down = executor->shutting_down;

				// in case of shutdown - we want queued (but not processed) tasks to be destroyed in main thread
				// so don't touch ctx->incoming_q here
				if (!shutting_down)
					local_q.swap(ctx->incoming_q);
			}

			if (shutting_down)
			{
				// cancel all executing tasks
				while (!ctx->running_q.empty())
				{
					async_task_ptr t = ctx->running_q.grab_front();
					t->task_abort();
				}

				// ev_async_stop(loop, ev);

				libev::break_loop(ctx->loop, EVUNLOOP_ALL); // just in case
				return;
			}

			while (!local_q.empty())
			{
				async_task_ptr task = local_q.grab_front();
				async_task_t *task_p = ctx->running_q.push_back(move(task));
				task_p->task_run(executor, loop);
			}
		}

		static void executor_wakeup(evloop_t *loop, evasync_t *ev, int revents)
		{
			auto *executor = MEOW_SELF_FROM_MEMBER(self_t, ev_, ev);

			queue_t local_q;
			{
				std::lock_guard<std::mutex> g_(executor->results_mtx);
				local_q.swap(executor->results_q);
			}

			while (!local_q.empty())
			{
				async_task_ptr task = local_q.grab_front();
				task->task_finished(loop);
			}
		}

		void thread_notify()
		{
			ev_async_send(thr_ctx_->loop.get(), &thr_ctx_->ev);
		}

		void executor_notify()
		{
			ev_async_send(loop_, &ev_);
		}

	private:

		struct thread_ctx_t
		{
			evloop_dynamic_t loop;
			evasync_t        ev;
			async_env_ptr    task_env;
			std::mutex       incoming_mtx;
			queue_t          incoming_q;
			queue_t          running_q;
		};

	private:
		std::unique_ptr<std::thread>   thr_;
		std::unique_ptr<thread_ctx_t>  thr_ctx_;

		evloop_t   *loop_;
		evasync_t   ev_;
		std::mutex  results_mtx;
		queue_t     results_q;

		uint64_t    shutting_down;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct async_scheduler___round_robin_t : private boost::noncopyable
	{
		void run_task(async_task_ptr task)
		{
			e_[e_current_]->run_task(move(task));
			if (++e_current_ >= e_.size())
				e_current_ = 0;
		}

	public:

		async_scheduler___round_robin_t()
			: e_current_(0)
		{
		}

		void add_executor(async_executor_ptr exec)
		{
			e_.push_back(move(exec));
		}

	private:
		std::vector<async_executor_ptr> e_;
		size_t                          e_current_;
	};

	typedef std::unique_ptr<async_scheduler___round_robin_t> async_scheduler_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////
