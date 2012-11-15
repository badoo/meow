////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_HPP_
#define MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_HPP_

#include <boost/noncopyable.hpp>

#include <meow/bitfield_union.hpp>
#include <meow/buffer.hpp>
#include <meow/buffer_chain.hpp>

#include <meow/libev/libev_fwd.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct io_context_t;

	struct generic_connection_t : private boost::noncopyable
	{
		virtual ~generic_connection_t() {}

	public: // general info

		virtual int                 fd() const = 0;
		virtual evloop_t*           loop() const = 0;

		virtual evio_t*             io_event() = 0;
		virtual io_context_t*       io_context() = 0;

	public: // flags

		struct flags_data_t
		{
			// the connection will be closed on next opportunity
			bool is_closing          : 1;

			// honor all pending writes before closing,
			//  ignored if "is_closing" is not set
			bool write_before_close  : 1;

			// if io_startup() was called more times than io_shutdown()
			bool io_started          : 1;
		};
		typedef meow::bitfield_union<flags_data_t, uint32_t> flags_t;

		flags_t flags;

	public: // io

		virtual void io_startup() = 0;   // activate full-duplex io_machine
		virtual void io_shutdown() = 0;  // stop io_machine
		virtual void io_reset() = 0;     // stop + close fd

		virtual void run_loop(int revents) = 0;
		inline void r_loop() { this->run_loop(EV_READ); }
		inline void w_loop() { this->run_loop(EV_WRITE); }
		inline void rw_loop() { this->run_loop(EV_READ | EV_WRITE); }
		inline void custom_loop() { this->run_loop(EV_CUSTOM); }

		virtual void activate(int revents) = 0;
		inline void r_activate() { this->activate(EV_READ); }
		inline void w_activate() { this->activate(EV_WRITE); }
		inline void rw_activate() { this->activate(EV_READ | EV_WRITE); }
		inline void custom_activate() { this->activate(EV_CUSTOM); }

		virtual void queue_buf(buffer_move_ptr) = 0;
		virtual void queue_chain(buffer_chain_t&) = 0;

		virtual void send(buffer_move_ptr) = 0;
		virtual void send_chain(buffer_chain_t&) = 0;

		virtual bool has_buffers_to_send() const = 0;

	public: // access to semi-private information for the brave

		virtual buffer_chain_t& wchain_ref() = 0;

	public: // closing

		inline bool is_closing() const { return flags->is_closing; }

		virtual void close_after_write() = 0;
		virtual void close_immediately() = 0;
		virtual void close_syncronously() = 0;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class EventsT>
	struct generic_connection_e_t : public generic_connection_t
	{
		typedef EventsT events_t;

		virtual events_t* ev() const = 0;
		virtual void      set_ev(events_t*) = 0;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_HPP_

