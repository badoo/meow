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
			evio_t 			io;
			evtimer_t 		timer;
			callback_t 		cb;

			item_t(int fd, callback_t const& cb)
				: cb(cb)
			{
				io.fd = fd;
			}

			int fd() const { return io.fd; }
			evio_t* io_event() { return &io; }
			evtimer_t* io_timer() { return &timer; }

			void store_self(self_t *self) { io_event()->data = self; }
			self_t* get_self() { return static_cast<self_t*>(io_event()->data); }

			void store_errno(int e)
			{
				// timeout's opaque data is always the errno store location
				//  which will be used on EV_CUSTOM activations
				io_timer()->data = union_cast<void*>(e);
			}

			int get_errno()
			{
				return union_cast<int>(io_timer()->data);
			}
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
				items_.pop_front();

				libev_cb_stage2(loop_, item, ECONNABORTED);
			}
		}

	private:

		virtual evloop_t* loop() const
		{
			return loop_;
		}

		virtual void try_connect(
				  callback_t const& 		cb
				, int 						fd
				, ipv4::address_t const& 	addr
				, os_timeval_t const& 		timeout
			)
		{
			item_move_ptr item(new item_t(fd, cb));
			item->store_self(this);

			this->do_connect(get_pointer(item), addr, timeout);

			items_.push_back(*item);
			item.release();
		}

		virtual void cancel_connect(int fd, bool do_callback)
		{
			item_t *item = this->find_fd(fd);
			if (NULL == item)
				return;

			if (do_callback)
				self_t::libev_cb_stage2(loop_, item, ECONNABORTED);
		}

	private:

		item_t* find_fd(int fd)
		{
			typedef list_t::iterator iterator_t;
			for (iterator_t i = items_.begin(); i != items_.end(); ++i)
			{
				item_t *item = &*i;
				if (item->fd() == fd)
					return item;
			}
			return NULL;
		}

		void do_connect(item_t *item, ipv4::address_t const& addr, os_timeval_t const& timeout)
		{
			// make sure it's nonblocking
			os_unix::nonblocking(item->fd());

			// start connecting
			struct sockaddr_in const& a = addr.sockaddr();

			if (log_)
				LOG_DEBUG_EX(log_, line_mode::prefix, "::connect({0}, {1}) ", item->fd(), addr);

			int n = ::connect(item->fd(), (struct sockaddr const*)&a, sizeof(a));

			if (log_)
				LOG_DEBUG_EX(log_, line_mode::suffix, "= {0}; errno: {1} : {2}", n, errno, strerror(errno));

			ev_io_init(item->io_event(), &self_t::libev_cb, item->fd(), EV_WRITE);
			ev_timer_init(item->io_timer(), &self_t::libev_timeout_cb, timeout.tv_sec, timeout.tv_usec);

			if (-1 == n)
			{
				if (EINPROGRESS != errno) // connection error that occured immediately
				{
					ev_feed_event(loop_, item->io_event(), EV_CUSTOM);
					item->store_errno(errno);
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
				item->store_errno(errno);
			}
		}

	private:

		static void libev_cb_stage2(evloop_t *loop, item_t *item, int err)
		{
			self_t *self = item->get_self();

			if (self->log_)
			{
				LOG_DEBUG_EX(self->log_, line_mode::prefix, "{0}; item: {1}, err: {2}", __func__, item, err);
				if (err)
					LOG_DEBUG_EX(self->log_, line_mode::suffix, " : {0}", strerror(err));
				else
					LOG_DEBUG_EX(self->log_, line_mode::suffix, "");
			}

			if (ev_is_active(item->io_timer()))
				ev_timer_stop(loop, item->io_timer());
			if (ev_is_active(item->io_event()))
				ev_io_stop(loop, item->io_event());

			item_move_ptr item_wrap(item);
			item->cb(item->fd(), err);
		}

		static void libev_cb(evloop_t *loop, evio_t *ev, int revents)
		{
			item_t *item = MEOW_SELF_FROM_MEMBER(item_t, io, ev);

			int err = (EV_CUSTOM & revents)
						? item->get_errno()
						: os_unix::getsockopt_ex<int>(item->fd(), SOL_SOCKET, SO_ERROR)
						;

			self_t::libev_cb_stage2(loop, item, err);
		}

		static void libev_timeout_cb(evloop_t *loop, evtimer_t *timer, int revents)
		{
			item_t *item = MEOW_SELF_FROM_MEMBER(item_t, timer, timer);
			self_t::libev_cb_stage2(loop, item, ETIMEDOUT);
		}

	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__FD_CONNECTOR_IMPL_HPP_

