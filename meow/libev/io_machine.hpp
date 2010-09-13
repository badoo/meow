////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <poll.h>

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>

#include <meow/str_ref.hpp>
#include <meow/smart_enum.hpp>
#include <meow/utility/bitmask.hpp>

#include <meow/format/inserter/integral.hpp>
#include <meow/format/inserter/pointer.hpp>

#include "io_context.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_SMART_ENUM(read_status, 
									((error, 	"error"))
									((closed, 	"closed"))
									((again, 	"again"))
									((full, 	"buffer_full"))
									);

	MEOW_DEFINE_SMART_ENUM(rd_consume_status, 
									((more, 	"more"))
									((finished, "finished"))
									((closed, 	"closed"))
									);

	MEOW_DEFINE_SMART_ENUM(write_status, 
									((error, 	"error"))
									((closed, 	"closed"))
									((again, 	"again"))
									((empty, 	"buffer_empty"))
									);

	MEOW_DEFINE_SMART_ENUM(wr_complete_status, 
									((more, 	"more"))
									((finished, "finished"))
									((closed, 	"closed"))
									);

	MEOW_DEFINE_SMART_ENUM(custom_op_status, 
									((more, 	"more"))
									((closed, 	"closed"))
									);

////////////////////////////////////////////////////////////////////////////////////////////////

#if 0 && IO_MACHINE_THIS_IS_AN_EXAMPLE_TRAITS_DEFINITIONS_YOU_CAN_USE

	struct iomachine_base_traits_def
	{
		typedef /* implementation-defined */ context_t;

		// get a pointer to event loop
		static evloop_t* ev_loop(context_t *ctx) {}

		// get changeable event object to use for io
		static io_context_t* io_context_ptr(context_t *ctx) {}

		// get context from the io, mirroring io_context_ptr()
		static context_t* context_from_io(io_context_t *io_ctx) {}

		// get io ops mask, if we want to read only, write only or both
		static int io_allowed_ops(context_t *ctx) {}

		// log a debug message from the engine
		int log_debug(context_t *ctx, char *fmt, ...) {}
	};

	struct iomachine_read_traits_def
	{
		// must return a buffer_ref pointing to memory area we can read into
		//  empty buffer_ref returned == cleanup and disallow reads till next iteration
		static buffer_ref read_get_buffer(context_t *ctx) {}

		// consume the read buffer,
		//  second param is the reference to filled part of the buffer
		//  that we got from read_get_buffer() call
		static rd_consume_status_t read_consume_buffer(context_t *ctx, buffer_ref read_part, read_status_t r_status) {}
	};

	struct iomachine_write_traits_def
	{
		// get next buffer to write
		// empty buffer_ref returned == cleanup and disallow reads till next iteration
		static buffer_ref write_get_buffer(context_t *ctx) {}

		// write complete
		//  called after writing as much a possible from the buffer we got from write_get_buffer()
		static wr_complete_status_t write_complete(context_t *ctx, buffer_ref written_part, write_status_t w_status) {}
	};

	struct iomachine_custom_operation_traits_def
	{
		// does the context need a custom operation executed now
		//  (used from inside event loop to check for custom op being set from inside it)
		static bool requires_custom_op(context_t *ctx) {}

		// execute a custom operation we woke up on
		static custom_op_status_t custom_operation(context_t *ctx) {}
	};

	//
	// example io_machine_t<> full templated definition
	//
	typedef io_machine_t<
			  client_connection_t
			, generic_traits_base_t
			, client_traits_read_t
			, generic_traits_write_t
			, generic_traits_custom_close_t
		> client_iomachine_t;

#endif // IO_MACHINE_THIS_IS_AN_EXAMPLE_TRAITS_DEFINITIONS_YOU_CAN_USE

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class ContextT 		// the context we'll be operating on
		, class BaseTraits 		// base traits, giving us global information, like evloop
		, class ReadTraits 		// read buffer alloc and consume
		, class WriteTraits 	// write buffer generation and post sent handling
		, class CustomOpTraits 	// custom activation handler
	>
	struct io_machine_t : private boost::noncopyable
	{
		typedef io_machine_t 	self_t;
		typedef ContextT 		context_t;

	private: // libev ops

		static void libev_cb(evloop_t *loop, evio_t *ev, int revents)
		{
			context_t *ctx = BaseTraits::context_from_io(io_context_t::cast_from_event(ev));
			self_t::cb(ctx, revents);
		}

		static void cb(context_t *ctx, int revents)
		{
			BaseTraits::log_debug(ctx, "{0}; ctx: {1}, revents: 0x{2}\n", __func__, ctx, meow::format::as_hex(revents));
			self_t::run_loop(ctx, revents);
		}

	private: // io ops

		struct read_result_t
		{
			read_status_t 	status;
			ssize_t 		filled_len;

			read_result_t(read_status_t s, ssize_t l)
				: status(s)
				, filled_len(l)
			{
			}
		};

		static read_result_t read_to_buffer(context_t *ctx, int fd, void *buf, size_t buf_len)
		{
			size_t curr_offset = 0;

			while (true)
			{
				BOOST_ASSERT(curr_offset <= buf_len);

				if (curr_offset == buf_len)
					return read_result_t(read_status::full, curr_offset);

				BaseTraits::log_debug(ctx, "::read({0}, {1} + {2}, {3} = {4} - {5}) = "
						, fd, buf, curr_offset
						, buf_len - curr_offset, buf_len, curr_offset
					);
				ssize_t n = ::read(fd, (char*)buf + curr_offset, buf_len - curr_offset);
				if (-1 == n)
					BaseTraits::log_debug(ctx, "{0}, errno: {1} : {2}\n", n, errno, strerror(errno));
				else
					BaseTraits::log_debug(ctx, "{0}\n", n);

				if (-1 == n)
				{
					return (EAGAIN == errno || EWOULDBLOCK == errno)
						? read_result_t(read_status::again, curr_offset)
						: read_result_t(read_status::error, curr_offset)
						;
				}
				else if (0 == n)
				{
					return read_result_t(read_status::closed, curr_offset);
				}

				BOOST_ASSERT(n > 0);

				curr_offset += n;
			}
		}

		struct write_result_t
		{
			write_status_t 	status;
			ssize_t 		bytes_written;

			write_result_t(write_status_t s, ssize_t l)
				: status(s)
				, bytes_written(l)
			{
			}
		};

		static write_result_t write_from_buffer(context_t *ctx, int fd, void *buf, size_t buf_len)
		{
			size_t curr_offset = 0;

			while (true)
			{
				BOOST_ASSERT(curr_offset <= buf_len);

				if (curr_offset == buf_len)
					return write_result_t(write_status::empty, curr_offset);

				BaseTraits::log_debug(ctx, "::write({0}, {1} + {2}, {3} = {4} - {5}) = "
						, fd, buf, curr_offset
						, buf_len - curr_offset, buf_len, curr_offset
					);
				ssize_t n = ::write(fd, (char*)buf + curr_offset, buf_len - curr_offset);
				if (-1 == n)
					BaseTraits::log_debug(ctx, "{0}, errno: {1} : {2}\n", n, errno, strerror(errno));
				else
					BaseTraits::log_debug(ctx, "{0}\n", n);

				if (-1 == n)
				{
					return (EAGAIN == errno || EWOULDBLOCK == errno)
						? write_result_t(write_status::again, curr_offset)
						: write_result_t(write_status::error, curr_offset)
						;
				}
				else if (0 == n)
				{
					return write_result_t(write_status::closed, curr_offset);
				}

				BOOST_ASSERT(n > 0);

				curr_offset += n;
			}
		}

		static bool fd_has_data_or_error(io_context_t *io_ctx)
		{
			struct pollfd pfd[] = { { io_ctx->fd(), POLLIN, 0 } };
			// poll with 0 timeout will just return if there is anything to read right now
			return poll(pfd, 1, 0);
		}

	public:

		static void prepare_context(context_t *ctx)
		{
			io_context_t *io_ctx = BaseTraits::io_context_ptr(ctx);
			ev_io_init(io_ctx->event(), &self_t::libev_cb, io_ctx->fd(), EV_NONE);
		}

		static void release_context(context_t *ctx)
		{
			io_context_t *io_ctx = BaseTraits::io_context_ptr(ctx);
			ev_io_stop(BaseTraits::ev_loop(ctx), io_ctx->event());
			ev_io_set(io_ctx->event(), io_ctx->fd(), EV_NONE);
		}

		static void activate_context(context_t *ctx, int io_requested_ops)
		{
			ev_feed_event(BaseTraits::ev_loop(ctx), BaseTraits::io_context_ptr(ctx)->event(), io_requested_ops);
		}

	public:

		static void custom_activate(context_t *ctx) { return self_t::activate_context(ctx, EV_CUSTOM); }
		static void r_activate(context_t *ctx) { return self_t::activate_context(ctx, EV_READ); }
		static void w_activate(context_t *ctx) { return self_t::activate_context(ctx, EV_WRITE); }
		static void rw_activate(context_t *ctx) { return self_t::activate_context(ctx, EV_READ | EV_WRITE); }

		static void custom_loop(context_t *ctx) { self_t::run_loop(ctx, EV_CUSTOM); }
		static void r_loop(context_t *ctx) { self_t::run_loop(ctx, EV_READ); }
		static void w_loop(context_t *ctx) { self_t::run_loop(ctx, EV_WRITE); }
		static void rw_loop(context_t *ctx) { self_t::run_loop(ctx, EV_READ | EV_WRITE); }

	private:

		// TODO: this is read-priority one
		//  can make many of these easily, but will need some duplicated code
		//  (or macros)
		static void run_loop(context_t *ctx, int const io_requested_ops = EV_READ | EV_WRITE)
		{
			// operations we're allowed to execute in this run
			int const io_allowed_ops = BaseTraits::io_allowed_ops(ctx);

			// the operations to be executed this run
			int io_executed_ops = (io_allowed_ops & io_requested_ops);
			if (EV_CUSTOM != io_requested_ops)
				io_executed_ops |= (EV_WRITE & io_allowed_ops);

			// operations we want to wait for, modified while looping
			int io_wait_ops = EV_NONE;

			// we need this everywhere and it's cached for an iteration
			//  might be a wrong thing to do tho, time will tell
			io_context_t *io_ctx = BaseTraits::io_context_ptr(ctx);

			BaseTraits::log_debug(ctx, "{0}; fd: {1}; aop: 0x{2}, rop: 0x{3}, eop: 0x{4}\n"
					, __func__, io_ctx->fd()
					, meow::format::as_hex(io_allowed_ops)
					, meow::format::as_hex(io_requested_ops)
					, meow::format::as_hex(io_executed_ops)
					);

			for (int io_current_ops = io_executed_ops; EV_NONE != io_current_ops; /**/)
			{
				BaseTraits::log_debug(ctx, "{0}; io_current_ops = 0x{1}, ev_ops: 0x{2}\n"
						, __func__
						, meow::format::as_hex(io_current_ops)
						, meow::format::as_hex(io_ctx->event()->events)
						);

				if (bitmask_test(io_current_ops, EV_CUSTOM))
				{
					custom_op_status_t c_status = CustomOpTraits::custom_operation(ctx);
					bitmask_clear(io_current_ops, EV_CUSTOM);

					switch (c_status)
					{
						case custom_op_status::more:
							break;
						case custom_op_status::closed:
							return;
					}
				}

				while (bitmask_test(io_current_ops, EV_READ)) // TODO: add iteration threshold here, via options prob
				{
					// peek if there is any data available at all
					//  so that we don't allocate huge buffers needlessly
					//  when connection is idle
					if (!self_t::fd_has_data_or_error(io_ctx))
					{
						BaseTraits::log_debug(ctx, "{0}; fd_has_data_or_error(): no available data on socket\n", __func__);
						bitmask_clear(io_current_ops, EV_READ);
						bitmask_set(io_wait_ops, EV_READ);
						break;
					}

					// now get a buffer
					meow::buffer_ref buf_to = ReadTraits::read_get_buffer(ctx);

					// don't read anymore till re-entering this function
					if (buf_to.empty())
					{
						BaseTraits::log_debug(ctx, "{0}; empty buffer from ReadTraits::read_get_buffer()\n", __func__);
						bitmask_clear(io_current_ops, EV_READ);
						bitmask_clear(io_wait_ops, EV_READ);
						break;
					}

					// do a read loop
					read_result_t r = self_t::read_to_buffer(ctx, io_ctx->fd(), buf_to.begin(), buf_to.size());

					// special 'again' handling
					if (read_status::again == r.status)
					{
						bitmask_clear(io_current_ops, EV_READ); // we don't want to read anymore, we got EAGAIN
						bitmask_set(io_wait_ops, EV_READ); // we want to wait for a read to be available again
					}

					// TODO: consume should be getting different status
					//  it's intrested in data + if connection is still alive,
					//   and it it isn't - it might care about why
					meow::buffer_ref filled_buf(buf_to.begin(), r.filled_len);
					rd_consume_status_t c_status = ReadTraits::read_consume_buffer(ctx, filled_buf, r.status);

					switch (c_status)
					{
						case rd_consume_status::finished: // cleanup everything, don't read anymore
							bitmask_clear(io_current_ops, EV_READ);
							bitmask_clear(io_wait_ops, EV_READ);
							break;
						case rd_consume_status::more: // read moar! but on next iteration after we write a bit
							bitmask_set(io_wait_ops, EV_READ);
							break;
						case rd_consume_status::closed: // fd has been closed
							return;
					}
				}

				if (CustomOpTraits::requires_custom_op(ctx))
				{
					bitmask_set(io_current_ops, EV_CUSTOM);
					continue;
				}

				while (bitmask_test(io_current_ops, EV_WRITE))
				{
					meow::buffer_ref buf = WriteTraits::write_get_buffer(ctx);

					if (buf.empty())
					{
						BaseTraits::log_debug(ctx, "{0}; empty buffer from WriteTraits::write_get_buffer()\n", __func__);
						bitmask_clear(io_current_ops, EV_WRITE);
						bitmask_clear(io_wait_ops, EV_WRITE);
						break;
					}
					write_result_t r = write_from_buffer(ctx, io_ctx->fd(), buf.data(), buf.size());

					if (write_status::again == r.status)
					{
						bitmask_clear(io_current_ops, EV_WRITE);
						bitmask_set(io_wait_ops, EV_WRITE);
					}

					meow::buffer_ref written_buf(buf.begin(), r.bytes_written);
					wr_complete_status_t c_status = WriteTraits::write_complete(ctx, written_buf, r.status);

					switch (c_status)
					{
						case wr_complete_status::finished: // cleanup everything, don't do stuff anymore
							bitmask_clear(io_current_ops, EV_WRITE);
							bitmask_clear(io_wait_ops, EV_WRITE);
							break;
						case wr_complete_status::more: // write moar! but on next iteration
							bitmask_set(io_wait_ops, EV_WRITE);
							break;
						case wr_complete_status::closed: // fd has been closed
							return;
					}
				}

				if (CustomOpTraits::requires_custom_op(ctx))
				{
					bitmask_set(io_current_ops, EV_CUSTOM);
					continue;
				}

			} // while

			// now adjust what we're waiting for based on what
			//  we were waiting for and on what we've been executing in this loop

			// io_wait_ops contains bits set for events we want to wait for
			//  bit limited only to the (io_requested_ops & io_allowed_ops)
			//  so we need to only change those bits in new value for wait

			int new_wait_ops = io_ctx->event()->events; 	// the initial mask is unchanged
			bitmask_clear(new_wait_ops, io_executed_ops); 	// clear out masked space
			bitmask_set(new_wait_ops, io_wait_ops); 		// set the new event bits

			BaseTraits::log_debug(
							  ctx
							, "{0}; fd: {1}; loop end; curr_ev_ops: 0x{2}, new_wait_ops = 0x{3}\n"
							, __func__
							, io_ctx->fd()
							, meow::format::as_hex(io_ctx->event()->events)
							, meow::format::as_hex(new_wait_ops)
							);
			if (io_ctx->event()->events != new_wait_ops)
			{
				evloop_t *loop = BaseTraits::ev_loop(ctx);

				ev_io_stop(loop, io_ctx->event());

				if (EV_NONE != new_wait_ops)
				{
					ev_io_set(io_ctx->event(), io_ctx->fd(), new_wait_ops);
					ev_io_start(loop, io_ctx->event());
				}
			}
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

