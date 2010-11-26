////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__LISTENER_IMPL_HPP_
#define MEOW_LIBEV__LISTENER_IMPL_HPP_

#include <boost/function.hpp>

#include <meow/api_call_error.hpp>
#include <meow/utility/offsetof.hpp>

#include <meow/libev/libev.hpp>

#include <meow/unix/fd_handle.hpp>
#include <meow/unix/fcntl.hpp>
#include <meow/unix/socket.hpp>

#include "listener.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev { namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		class CallbackPolicy // CallbackPolicy implements a single function
							 //  static void listener_callback(listener_t*, int new_socket)
	>
	struct listener_static_impl_t : public listener_t
	{
		typedef listener_static_impl_t self_t;

		listener_static_impl_t(libev::evloop_t *loop)
			: loop_(loop)
		{
			ev_init(&evt_, &libev_cb);
		}

		~listener_static_impl_t()
		{
			this->do_shutdown();
		}

	private:

		virtual int fd() const { return event()->fd; }
		virtual evio_t const* event() const { return &evt_; }
		virtual evloop_t* loop() const { return loop_; }

		virtual void start(ipv4::address_t const& addr)
		{
			if (ev_is_active(&evt_))
				this->do_shutdown();

			this->do_start(addr);
		}

		virtual void shutdown()
		{
			this->do_shutdown();
		}

	private:

		void do_start(ipv4::address_t const& addr)
		{
			os_unix::fd_handle_t s(os_unix::socket_ex(PF_INET, SOCK_STREAM, 0));
			os_unix::setsockopt_ex(get_handle(s), SOL_SOCKET, SO_REUSEADDR, int(~0));
			os_unix::bind_ex(get_handle(s), (sockaddr*)addr.sockaddr_tmp(), addr.addrlen());
			os_unix::listen_ex(get_handle(s), -1);
			os_unix::nonblocking(get_handle(s));

			ev_io_set(&evt_, get_handle(s), EV_READ);
			ev_io_start(loop_, &evt_);

			s.release();
		}

		void do_shutdown()
		{
			ev_io_stop(loop_, &evt_);
			close(evt_.fd);
			loop_ = NULL;
		}

	private:

		static void libev_cb(libev::evloop_t*, libev::evio_t *ev, int revents)
		{
			self_t *self = MEOW_SELF_FROM_MEMBER(self_t, evt_, ev);
			self->cb(revents);
		}

		void cb(int revents)
		{
			if (EV_READ & revents)
			{

				while (true)
				{
					int new_sock = ::accept(evt_.fd, NULL, NULL);
					if (-1 == new_sock)
					{
						if (EAGAIN == errno)
							break;
						else
							throw meow::api_call_error("accept");
					}

					CallbackPolicy::listener_callback(this, new_sock);
				}
			}
		}

	private:
		libev::evio_t 	evt_;
		libev::evloop_t *loop_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct listener_dynamic_impl_t
		: public listener_static_impl_t<listener_dynamic_impl_t>
	{
		typedef listener_dynamic_impl_t self_t;
		typedef listener_static_impl_t<listener_dynamic_impl_t> base_t;

		typedef boost::function<void(listener_t*, int)> callback_t;

		listener_dynamic_impl_t(libev::evloop_t *loop, callback_t const& cb)
			: base_t(loop)
			, callback_(cb)
		{
		}

		static void listener_callback(base_t *l, int s)
		{
			self_t *self = static_cast<self_t*>(l);
			self->callback_(l, s);
		}

	private:
		callback_t callback_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef detail::listener_dynamic_impl_t::callback_t listener_callback_t;

	template<class L>
	inline listener_move_ptr start_listener(L& loop, ipv4::address_t const& addr, listener_callback_t const& cb)
	{
		listener_move_ptr l(new detail::listener_dynamic_impl_t(get_handle(loop), cb));
		l->start(addr);
		return move(l);
	}

	template<class L, class CallbackPolicy>
	inline listener_move_ptr start_listener(L& loop, ipv4::address_t const& addr)
	{
		listener_move_ptr l(new detail::listener_static_impl_t<CallbackPolicy>(get_handle(loop)));
		l->start(addr);
		return move(l);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__LISTENER_IMPL_HPP_

