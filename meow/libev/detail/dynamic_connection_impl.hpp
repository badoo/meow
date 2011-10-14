////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__DYNAMIC_CONNECTION_IMPL_HPP_
#define MEOW_LIBEV_DETAIL__DYNAMIC_CONNECTION_IMPL_HPP_

#include <meow/libev/detail/dynamic_connection.hpp>
#include <meow/libev/detail/generic_connection_impl.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits>
	struct dynamic_connection_repack_traits : public Traits
	{
		struct read
		{
			struct context_t {};

			template<class ConnectionT>
			static buffer_ref get_buffer(ConnectionT *c)
			{
				return c->ev()->get_buffer(c);
			}

			template<class ConnectionT>
			static rd_consume_status_t consume_buffer(ConnectionT *c, buffer_ref b, read_status_t r_status)
			{
				// now we might have our connection dead already
				if (read_status::error == r_status)
				{
					c->ev()->on_closed(c, io_close_report(io_close_reason::io_error, errno));
					return rd_consume_status::closed;
				}

				// we might have data read here,
				//  try processing it regardless of closed state first
				bool const is_closed = (read_status::closed == r_status);
				rd_consume_status_t const buf_status = c->ev()->consume_buffer(c, b, is_closed);

				// okay, buffer has been processed, so check the connection state

				// first, the connection state itself, if it's closed
				//  we can't do much else other than closing it on our end too
				if (read_status::closed == r_status)
				{
					c->ev()->on_closed(c, io_close_report(io_close_reason::peer_close));
					return rd_consume_status::closed;
				}

				// next is the buffer handler, if it says we're closed - then we are
				if (rd_consume_status::closed == buf_status)
				{
					c->ev()->on_closed(c, io_close_report(io_close_reason::custom_close));
					return rd_consume_status::closed;
				}

				return buf_status;
			}
		};
	};

	template<
		  class Traits
		, class Interface = dynamic_connection_t
		>
	struct dynamic_connection_impl_t
		: public generic_connection_impl_t<Interface, dynamic_connection_repack_traits<Traits> >
	{
		typedef generic_connection_impl_t<Interface, dynamic_connection_repack_traits<Traits> > impl_t;
		typedef typename impl_t::events_t  events_t;

		dynamic_connection_impl_t(evloop_t *loop, int fd, events_t *ev = NULL)
			: impl_t(loop, fd, ev)
		{
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__DYNAMIC_CONNECTION_IMPL_HPP_

