////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_HPP_
#define MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_HPP_

#include <boost/noncopyable.hpp>

#include <meow/buffer.hpp>
#include <meow/buffer_chain.hpp>
#include <meow/bitfield_union.hpp>

#include <meow/libev/libev_fwd.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct generic_connection_close_data_t
	{
		bool after_write : 1;
		bool immediately : 1;
	};
	typedef meow::bitfield_union<generic_connection_close_data_t> close_flags_t;

////////////////////////////////////////////////////////////////////////////////////////////////

	struct generic_connection_t : private boost::noncopyable
	{
		virtual ~generic_connection_t() {}

	public: // general info

		virtual int 		fd() const = 0;
		virtual evloop_t* 	loop() const = 0;

	public: // io

		virtual void r_loop() = 0;
		virtual void w_loop() = 0;
		virtual void rw_loop() = 0;

		virtual void activate(int revents) = 0;
		virtual void r_activate() = 0;
		virtual void w_activate() = 0;
		virtual void rw_activate() = 0;
		virtual void custom_activate() = 0;

		virtual void queue_buf(buffer_move_ptr) = 0;
		virtual void queue_chain(buffer_chain_t&) = 0;

		virtual void send(buffer_move_ptr) = 0;
		virtual void send_chain(buffer_chain_t&) = 0;

	public: // access to semi-private information for the brave

		virtual buffer_chain_t& wchain_ref() = 0;

	public: // closing

		virtual bool 			is_closing() const = 0;
		virtual close_flags_t 	close_flags() const = 0;

		virtual void close_after_write() = 0;
		virtual void close_immediately() = 0;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_HPP_

