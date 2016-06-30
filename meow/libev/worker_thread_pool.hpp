////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>

#include <boost/noncopyable.hpp>

#include <meow/std_unique_ptr.hpp>
#include <meow/ptr_list.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct worker_item_t : public meow::ptr_list_hook_t
	{
		virtual ~worker_item_t() {}
		virtual bool is_shutdown() const = 0;
		virtual void work()              = 0;
		virtual void callback()          = 0;
	};
	typedef std::unique_ptr<worker_item_t> worker_item_ptr;

	template<class T, class WF, class CF>
	struct worker_item_impl_t : public worker_item_t
	{
		using state_t = typename std::remove_reference<T>::type;

		worker_item_impl_t(T&& state, WF const& work, CF const& callback)
			: state_(std::move(state))
			, work_(work)
			, callback_(callback)
		{
		}

		virtual bool is_shutdown() const { return false; }
		virtual void work()              { work_(state_); }
		virtual void callback()          { callback_(state_); }

	private:
		state_t state_;
		WF      work_;
		CF      callback_;
	};

	struct worker_item_shutdown_t : public worker_item_t
	{
		virtual bool is_shutdown() const { return true; }
		virtual void work()              {}
		virtual void callback()          {}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	using libev::evloop_t;
	using libev::evasync_t;

	struct workers_t
	{
		using queue_t = meow::ptr_list_t<worker_item_t>;

		workers_t(evloop_t *l)
			: loop(l)
			, shutting_down(0)
		{
		}

		evloop_t                 *loop;
		evasync_t                 ev;
		uint64_t                  shutting_down;

		queue_t                   r_queue;
		std::mutex                r_lock;

		std::vector<std::thread>  t;
		queue_t                   w_queue;
		std::mutex                w_lock;
		std::condition_variable   w_cond;
	};
	typedef std::unique_ptr<workers_t> workers_ptr;

	inline workers_ptr workers_init(evloop_t *loop, size_t n)
	{
		auto ctx = workers_ptr { new workers_t (loop) };

		auto const thr_wakeup = [](evloop_t *loop, evasync_t *ev, int revents)
		{
			auto parent = static_cast<workers_t*>(ev->data);

			// ff::fmt(stdout, "thr wakeup {0}\n", parent);

			workers_t::queue_t q;
			{
				std::lock_guard<std::mutex> g_{ parent->r_lock };
				q.swap(parent->r_queue);
			}

			while (!q.empty())
			{
				auto item = q.grab_front();
				item->callback();
			}
		};

		auto parent = ctx.get();

		ctx->loop = loop;

		ctx->ev.data = parent;
		ev_async_init(&ctx->ev, thr_wakeup);
		ev_async_start(loop, &ctx->ev);

		for (size_t i = 0; i < n; i++)
		{
			ctx->t.emplace_back(std::thread { [parent]() {
				while (true)
				{
					auto item = [parent]() {
						std::unique_lock<std::mutex> lk_ { parent->w_lock };
						parent->w_cond.wait(lk_, [&]() { return !parent->w_queue.empty(); });
						return parent->w_queue.grab_front(); // grab stuff from the queue and unlock in dtor
					}();

					if (item->is_shutdown())
						break;

					item->work();

					{
						std::lock_guard<std::mutex> g_{ parent->r_lock };
						parent->r_queue.push_back(move(item));
					}

					ev_async_send(parent->loop, &parent->ev);
				}
			} });
		}

		return move(ctx);
	}

	template<class T, class WF, class CF>
	inline void workers_schedule(workers_t& ctx, T&& state, WF const& work, CF const& callback)
	{
		auto item = meow::make_unique<worker_item_impl_t<T, WF, CF>>(std::forward<T>(state), work, callback);
		auto parent = &ctx;

		{
			std::lock_guard<std::mutex> g_{ parent->w_lock };
			if (parent->shutting_down != 0)
				return;

			parent->w_queue.push_back(move(item));
		}

		parent->w_cond.notify_one();
	}

	inline void workers_shutdown(workers_t& ctx)
	{
		auto parent = &ctx;

		{
			std::lock_guard<std::mutex> g_{ parent->w_lock };

			parent->shutting_down = 1;

			// send shutdown messages
			for (size_t i = 0; i < parent->t.size(); i++)
			{
				// ff::fmt(stdout, "sending shutdown signal {0}\n", i);
				parent->w_queue.push_back(meow::make_unique<worker_item_shutdown_t>());
			}
		}

		for (size_t i = 0; i < parent->t.size(); i++)
		{
			parent->w_cond.notify_one(); // would notify_all() work here ?
		}

		// wait for threads to finish
		for (size_t i = 0; i < ctx.t.size(); i++)
		{
			// ff::fmt(stdout, "joining thread {0}\n", i);
			parent->t[i].join();
		}

		// ff::fmt(stdout, "workers_stop done\n");
	}


////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow
////////////////////////////////////////////////////////////////////////////////////////////////
