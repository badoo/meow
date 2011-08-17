////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__FD_CONNECTOR_IMPL_HPP_
#define MEOW_LIBEV__FD_CONNECTOR_IMPL_HPP_

#include <boost/intrusive/list.hpp>

#include <meow/libev/libev.hpp>

#include <meow/logging/logger.hpp>
#include <meow/logging/log_write.hpp>

#include <meow/unix/fcntl.hpp> 	// nonblocking
#include <meow/unix/ipv4_address.hpp>
#include <meow/unix/socket.hpp> // getsockopt_ex
#include <meow/unix/time.hpp> 	// os_timeval_t

#include <meow/utility/offsetof.hpp>
#include <meow/convert/union_cast.hpp>

#include "fd_connector.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	namespace bi = boost::intrusive;

	struct fd_connector_impl_t : public fd_connector_t
	{
		typedef fd_connector_impl_t self_t;

		struct item_t;
		typedef bi::list_base_hook<bi::link_mode<bi::auto_unlink> > hook_t;
		typedef bi::list<
					  item_t
					, bi::base_hook<hook_t>
					, bi::constant_time_size<false>
				> list_t;

		struct item_t : public hook_t
		{
			io_context_ptr  io_ctx;   // io, will be given out, ->data points to item_t
			evtimer_t 		timer;    // timeout, ->data points to fd_connector_impl_t
			int             err_code; // saved errno from ::connect()
			callback_t 		cb;

			item_t(int fd, callback_t const& cb)
				: io_ctx(new io_context_t(fd))
				, cb(cb)
			{
				store_item_ptr(this);
			}

			int fd() const { return io_ctx->fd(); }
			evio_t* io_event() { return io_ctx->event(); }
			evtimer_t* io_timer() { return &timer; }

			static void store_item_ptr(item_t *item) { item->io_ctx->reset_data(item); }
			static item_t* get_item_ptr(io_context_t *ctx) { return static_cast<item_t*>(ctx->data()); }

			void store_connector_ptr(self_t *self) { io_timer()->data = self; }
			self_t* get_connector_ptr() { return static_cast<self_t*>(io_timer()->data); }
		};
		typedef boost::static_move_ptr<item_t> item_move_ptr;

		typedef logging::logger_t logger_t;

	private:
		evloop_t 	*loop_;
		logger_t 	*log_;
		list_t 		items_;

	public:

		fd_connector_impl_t(evloop_t *l, logger_t *log = NULL)
			: loop_(l)
			, log_(log)
		{
		}

		~fd_connector_impl_t()
		{
			while (!items_.empty())
			{
				item_t *item = &items_.front();
				item_move_ptr item_wrap = this->item_grab_from_list(item);
				this->item_do_callback(item, ECONNABORTED);
			}
		}

	private:

		virtual evloop_t* loop() const
		{
			return loop_;
		}

		virtual token_t try_connect(
				  callback_t const& 		cb
				, int 						fd
				, ipv4::address_t const& 	addr
				, os_timeval_t const& 		timeout
			)
		{
			item_move_ptr item(new item_t(fd, cb));
			item->store_connector_ptr(this);

			this->do_connect(get_pointer(item), addr, timeout);

			items_.push_back(*item.release());
			return reinterpret_cast<token_t>(&items_.back());
		}

		virtual void cancel_connect(token_t token, bool do_callback)
		{
			if (NULL == token)
				return;

			item_t *item = reinterpret_cast<item_t*>(token);
			item_move_ptr item_wrap = this->item_grab_from_list(item);

			if (do_callback)
				this->item_do_callback(item, ECONNABORTED);
		}

	private:

		void do_connect(item_t *item, ipv4::address_t const& addr, os_timeval_t const& timeout)
		{
			// make sure it's nonblocking
			os_unix::nonblocking(item->fd());

			// nothrow after this line
			// except if logging throws, but then we're fucked

			// start connecting
			struct sockaddr_in const& a = addr.sockaddr();

			if (log_)
				LOG_DEBUG_EX(log_, line_mode::prefix, "::connect({0}, {1}) ", item->fd(), addr);

			int n = ::connect(item->fd(), (struct sockaddr const*)&a, sizeof(a));

			if (log_)
				LOG_DEBUG_EX(log_, line_mode::suffix, "= {0}; errno: {1} : {2}", n, errno, strerror(errno));

			ev_io_init(item->io_event(), &self_t::libev_cb, item->fd(), EV_WRITE);
			ev_timer_init(item->io_timer(), &self_t::libev_timeout_cb, os_timeval_to_double(timeout), 0.);

			if (-1 == n)
			{
				if (EINPROGRESS != errno) // connection error that occured immediately
				{
					ev_feed_event(loop_, item->io_event(), EV_CUSTOM);
					item->err_code = errno;
				}
				else
				{
					ev_io_start(loop_, item->io_event());
					ev_timer_start(loop_, item->io_timer());
				}
			}
			else // connection established immediately
			{
				BOOST_ASSERT((0 == n) && "connect() returns either -1 or 0");
				ev_feed_event(loop_, item->io_event(), EV_CUSTOM);
				item->err_code = 0;
			}
		}

	private: // item ops

		// take the item from the list for clean disposal later
		//  the returned result is guaranteed to not be engaged with libev in anyway
		item_move_ptr item_grab_from_list(item_t *item)
		{
			ev_timer_stop(loop_, item->io_timer());
			ev_io_stop(loop_, item->io_event());

			items_.erase(items_.iterator_to(*item));
			return boost::move_raw(item);
		}

		void item_do_callback(item_t *item, int err)
		{
			BOOST_ASSERT(this == item->get_connector_ptr());

			if (log_)
			{
				LOG_DEBUG_EX(log_, line_mode::prefix, "fd_connector::{0}; fd: {1}, err: {2}", __func__, item->fd(), err);
				if (err)
					LOG_DEBUG_EX(log_, line_mode::suffix, " : {0}", strerror(err));
				else
					LOG_DEBUG_EX(log_, line_mode::suffix, "");
			}

			// need the ref to pass it through boost::function
			io_context_ptr& io_ctx = item->io_ctx;
			item->cb(io_ctx, err);
		}

	private:

		void cb(item_t *item, int err)
		{
			item_move_ptr item_wrap = item_grab_from_list(item);
			this->item_do_callback(item, err);
		}

		static void libev_cb(evloop_t *loop, evio_t *ev, int revents)
		{
			io_context_t *io_ctx = io_context_t::cast_from_event(ev);
			item_t *item = item_t::get_item_ptr(io_ctx);
			self_t *self = item->get_connector_ptr();

			int err = (EV_CUSTOM & revents)
						? item->err_code
						: os_unix::getsockopt_ex<int>(item->fd(), SOL_SOCKET, SO_ERROR)
						;

			self->cb(item, err);
		}

		static void libev_timeout_cb(evloop_t *loop, evtimer_t *timer, int revents)
		{
			item_t *item = MEOW_SELF_FROM_MEMBER(item_t, timer, timer);
			self_t *self = item->get_connector_ptr();

			self->cb(item, ETIMEDOUT);
		}

	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__FD_CONNECTOR_IMPL_HPP_

