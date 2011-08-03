////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__IO_MACHINE_HPP_
#define MEOW_LIBEV__IO_MACHINE_HPP_

#include <cerrno>
#include <poll.h>

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>

#include <meow/str_ref.hpp>
#include <meow/smart_enum.hpp>
#include <meow/utility/bitmask.hpp>
#include <meow/utility/line_mode.hpp>
#include <meow/utility/nested_type_checker.hpp>

#include <meow/format/format.hpp>
#include <meow/format/format_tmp.hpp>
#include <meow/format/inserter/integral.hpp>
#include <meow/format/inserter/pointer.hpp>

#include "io_context.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
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

	template<class ContextT>
	struct iomachine_example_traits
	{
		typedef ContextT context_t;

		struct base
		{
			// get a pointer to event loop
			static evloop_t* ev_loop(context_t *ctx) {}

			// get changeable event object to use for io
			static io_context_t* io_context_ptr(context_t *ctx) {}

			// get context from the io, mirroring io_context_ptr()
			static context_t* context_from_io(io_context_t *io_ctx) {}
		};

		struct read
		{
			// must return a buffer_ref pointing to memory area we can read into
			//  empty buffer_ref returned == cleanup and disallow reads till next iteration
			static buffer_ref get_buffer(context_t *ctx) {}

			// consume the read buffer,
			//  second param is the reference to filled part of the buffer
			//  that we got from read_get_buffer() call
			static rd_consume_status_t consume_buffer(context_t *ctx, buffer_ref read_part, read_status_t r_status) {}
		};

		struct write
		{
			// get next buffer to write
			// empty buffer_ref returned == cleanup and disallow reads till next iteration
			static buffer_ref get_buffer(context_t *ctx) {}

			// write complete
			//  called after writing as much a possible from the buffer we got from write_get_buffer()
			static wr_complete_status_t write_complete(context_t *ctx, buffer_ref written_part, write_status_t w_status) {}
		};

		struct allowed_ops // optional
		{
			// get io ops mask, if we want to read only, write only or both
			// DEFAULT IF UNSET: (EV_READ | EV_WRITE | EV_CUSTOM)
			static int allowed_ops(context_t *ctx) {}
		};

		struct custom_op // optional
		{
			// does the context need a custom operation executed now
			//  (used from inside event loop to check for custom op being set from inside it)
			static bool requires_custom_op(context_t *ctx) {}

			// execute a custom operation we woke up on
			static custom_op_status_t custom_operation(context_t *ctx) {}
		};

		struct read_precheck // optional
		{
			// checks if there is an error or data that can be read from the socket
			// returns true if there is
			// used to avoid allocating read buffers before there is actually data available
			// poor man's accept-filter-ish functionality
			// NOTICE:  use iomachine_read_precheck_do_poll_t if you want to do the checking
			// 				with poll({fd, POLLIN, 0}, 1, timeout = 0)
			// DEFAULT: return true
			static bool has_data_or_error(context_t *ctx, io_context_t *io_ctx) {}
		};

		struct log_writer // optional
		{
			// ask if the log messages are enabled now
			static bool is_allowed(context_t *ctx) {}

			// log a message from the engine
			static void write(context_t *ctx, line_mode_t lmode, char *fmt, ...) {}
		};

		struct activity_tracker // optional
		{
			// idle tracking init, called when context is prepared
			static void init(context_t *ctx) {}

			// idle tracking free resources, called when context is released
			static void deinit(context_t *ctx) {}

			// idle tracking activity notification,
			//  called every time exiting run_loop()
			//  if read_consume_buffer() OR write_complete() were called
			static void on_activity(context_t *ctx, int revents) {}
		};
	};

	//
	// example io_machine_t<> definition
	//
	typedef io_machine_t<
			  client_connection_t
			, generic_connection_simplest_traits<client_connection_t>
		> client_iomachine_t;

#endif // IO_MACHINE_THIS_IS_AN_EXAMPLE_TRAITS_DEFINITIONS_YOU_CAN_USE

////////////////////////////////////////////////////////////////////////////////////////////////
// this thing checks for presence of subtype nested_name in the template param Tr
// and, assuming the presence of thunk_t<bool,type> available around
// typedefs it with proper template args as thunk

#define DEFINE_THUNK(nested_name) 											\
	MEOW_DEFINE_NESTED_TYPE_CHECKER(check_impl_available, nested_name); 	\
	template<bool enabled, class Tr> struct type_thunk_t; 					\
	template<class Tr> struct type_thunk_t<true, Tr> { 						\
		typedef typename Tr::nested_name type; 								\
		enum { is_enabled = !boost::is_same<void, type>::value };			\
	};																		\
	template<class Tr> struct type_thunk_t<false, Tr> {						\
		typedef Tr type;													\
		enum { is_enabled = false };										\
	}; 																		\
	typedef type_thunk_t<check_impl_available<Traits>::value, Traits> tth_t;	\
	typedef thunk_t<tth_t::is_enabled, typename tth_t::type> thunk;			\
/**/

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ContextT, class Traits>
	struct iomachine_allowed_ops_wrap_t
	{
		template<bool enabled, class Tr>
		struct thunk_t;

		template<class Tr> struct thunk_t<true, Tr>
		{
			static int get(ContextT *ctx) { return Tr::get(ctx); }
		};

		template<class Tr> struct thunk_t<false, Tr>
		{
			static int get(ContextT *) { return EV_READ | EV_WRITE | EV_CUSTOM; }
		};

		DEFINE_THUNK(allowed_ops);

		static int get(ContextT *ctx) { return thunk::get(ctx); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ContextT, class Traits>
	struct iomachine_custom_op_wrap_t
	{
		template<bool enabled, class Tr>
		struct thunk_t;

		template<class Tr> struct thunk_t<true, Tr>
		{
			static bool requires_custom_op(ContextT *ctx) { return Tr::requires_custom_op(ctx); }
			static custom_op_status_t custom_operation(ContextT *ctx) { return Tr::custom_operation(ctx); }
		};

		template<class Tr> struct thunk_t<false, Tr>
		{
			static bool requires_custom_op(ContextT *) { return false; }
			static custom_op_status_t custom_operation(ContextT *) { return custom_op_status::more; }
		};

		DEFINE_THUNK(custom_op);

		static bool requires_custom_op(ContextT *ctx) { return thunk::requires_custom_op(ctx); }
		static custom_op_status_t custom_operation(ContextT *ctx) { return thunk::custom_operation(ctx); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ContextT, class Traits>
	struct iomachine_read_precheck_wrap_t
	{
		template<bool enabled, class Tr>
		struct thunk_t;

		template<class Tr> struct thunk_t<true, Tr>
		{
			static bool has_data_or_error(ContextT *ctx, io_context_t *io_ctx) { return Tr::has_data_or_error(ctx, io_ctx); }
		};

		template<class Tr> struct thunk_t<false, Tr>
		{
			static bool has_data_or_error(ContextT *ctx, io_context_t *io_ctx) { return true; }
		};

		DEFINE_THUNK(read_precheck);

		static bool has_data_or_error(ContextT *ctx, io_context_t *io_ctx) { return thunk::has_data_or_error(ctx, io_ctx); }
	};

	// the implementation to use in app code if the precheck is desirable
	struct iomachine_read_precheck_do_poll_t
	{
		template<class ContextT>
		static bool has_data_or_error(ContextT *ctx, io_context_t *io_ctx)
		{
			struct pollfd pfd[] = { { io_ctx->fd(), POLLIN | POLLPRI, 0 } };
			// poll with 0 timeout will just return if there is anything to read right now
			return poll(pfd, 1, 0);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ContextT, class Traits>
	struct iomachine_log_writer_wrap_t
	{
		template<bool enabled, class Tr>
		struct thunk_t;

		template<class Tr> struct thunk_t<true, Tr>
		{
			static bool is_allowed(ContextT *ctx) { return Traits::log_writer::is_allowed(ctx); }

			#define DEFINE_IOMACHINE_LOG_WRITER_FMT_FUNCTION_TRUE(z, n, d) 					\
				template<class F FMT_TEMPLATE_PARAMS(n)> 									\
				static void write( 															\
						  ContextT *ctx 													\
						, line_mode_t lmode 												\
						, F const& fmt 														\
						  FMT_DEF_PARAMS(n)) 												\
				{ 																			\
					Tr::write( 																\
							  ctx 															\
							, lmode 														\
							, format::fmt_tmp<512>(fmt FMT_CALL_SITE_ARGS(n)) 				\
							); 																\
				} 																			\
			/**/
			BOOST_PP_REPEAT(32, DEFINE_IOMACHINE_LOG_WRITER_FMT_FUNCTION_TRUE, _);
		};

		template<class Tr> struct thunk_t<false, Tr>
		{
			static bool is_allowed(ContextT *ctx) { return false; }

			#define DEFINE_IOMACHINE_LOG_WRITER_FMT_FUNCTION_FALSE(z, n, d) 				\
				template<class F FMT_TEMPLATE_PARAMS(n)> 									\
				static void write( 															\
						  ContextT *ctx 													\
						, line_mode_t lmode 												\
						, F const& fmt 														\
						  FMT_DEF_PARAMS(n)) 												\
				{ 																			\
				} 																			\
			/**/
			BOOST_PP_REPEAT(32, DEFINE_IOMACHINE_LOG_WRITER_FMT_FUNCTION_FALSE, _);
		};

		DEFINE_THUNK(log_writer);

		static bool is_allowed(ContextT *ctx) { return thunk::is_allowed(ctx); }

		#define DEFINE_IOMACHINE_LOG_WRITER_FMT_FUNCTION(z, n, d) 				\
			template<class F FMT_TEMPLATE_PARAMS(n)> 									\
			static void write( 															\
					  ContextT *ctx 													\
					, line_mode_t lmode 												\
					, F const& fmt 														\
					  FMT_DEF_PARAMS(n)) 												\
			{ 																			\
				thunk::write(ctx, lmode, fmt FMT_CALL_SITE_ARGS(n)); 					\
			} 																			\
		/**/
		BOOST_PP_REPEAT(32, DEFINE_IOMACHINE_LOG_WRITER_FMT_FUNCTION, _);
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ContextT, class Traits>
	struct iomachine_activity_tracker_wrap_t
	{
		template<bool enabled, class Tr>
		struct thunk_t;

		template<class Tr> struct thunk_t<true, Tr>
		{
			static void init(ContextT *ctx) { Tr::init(ctx); }
			static void deinit(ContextT *ctx) { Tr::deinit(ctx); }
			static void on_activity(ContextT *ctx, int revents) { Tr::on_activity(ctx, revents); }
		};

		template<class Tr> struct thunk_t<false, Tr>
		{
			static void init(ContextT *ctx) {}
			static void deinit(ContextT *ctx) {}
			static void on_activity(ContextT *ctx, int revents) {}
		};

		DEFINE_THUNK(activity_tracker);

		static void init(ContextT *ctx) { thunk::init(ctx); }
		static void deinit(ContextT *ctx) { thunk::deinit(ctx); }
		static void on_activity(ContextT *ctx, int revents) { thunk::on_activity(ctx, revents); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

#undef DEFINE_THUNK

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class ContextT
		, class Traits
	>
	struct io_machine_t : private boost::noncopyable
	{
		typedef io_machine_t 	self_t;
		typedef ContextT 		context_t;

		typedef typename Traits::base 	tr_base;
		typedef typename Traits::read 	tr_read;
		typedef typename Traits::write 	tr_write;

		typedef iomachine_allowed_ops_wrap_t<ContextT, Traits> 			tr_allowed_ops;
		typedef iomachine_custom_op_wrap_t<ContextT, Traits> 			tr_custom_op;
		typedef iomachine_read_precheck_wrap_t<ContextT, Traits>		tr_read_precheck;
		typedef iomachine_log_writer_wrap_t<ContextT, Traits> 			tr_log;
		typedef iomachine_activity_tracker_wrap_t<ContextT, Traits> 	tr_activity;

	private: // libev ops

		static void libev_cb(evloop_t *loop, evio_t *ev, int revents)
		{
			context_t *ctx = tr_base::context_from_io(io_context_t::cast_from_event(ev));
			self_t::cb(ctx, revents);
		}

		static void cb(context_t *ctx, int revents)
		{
//			tr_log::write(ctx, line_mode::single, "{0}; ctx: {1}, revents: 0x{2}", __func__, ctx, meow::format::as_hex(revents));
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

				if (tr_log::is_allowed(ctx))
				{
					tr_log::write(ctx, line_mode::prefix, "::read({0}, {1} + {2}, {3} = {4} - {5}) = "
							, fd, buf, curr_offset
							, buf_len - curr_offset, buf_len, curr_offset
						);
				}

				ssize_t n = ::read(fd, (char*)buf + curr_offset, buf_len - curr_offset);

				if (tr_log::is_allowed(ctx))
				{
					if (-1 == n)
						tr_log::write(ctx, line_mode::suffix, "{0}, errno: {1} : {2}", n, errno, strerror(errno));
					else
						tr_log::write(ctx, line_mode::suffix, "{0}", n);
				}

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

				if (tr_log::is_allowed(ctx))
				{
					tr_log::write(ctx, line_mode::prefix, "::write({0}, {1} + {2}, {3} = {4} - {5}) = "
							, fd, buf, curr_offset
							, buf_len - curr_offset, buf_len, curr_offset
						);
				}

				ssize_t n = ::write(fd, (char*)buf + curr_offset, buf_len - curr_offset);

				if (tr_log::is_allowed(ctx))
				{
					if (-1 == n)
						tr_log::write(ctx, line_mode::suffix, "{0}, errno: {1} : {2}", n, errno, strerror(errno));
					else
						tr_log::write(ctx, line_mode::suffix, "{0}", n);
				}

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

	public:

		static void prepare_context(context_t *ctx)
		{
			io_context_t *io_ctx = tr_base::io_context_ptr(ctx);
			ev_io_init(io_ctx->event(), &self_t::libev_cb, io_ctx->fd(), EV_NONE);

			tr_activity::init(ctx);
		}

		static void release_context(context_t *ctx)
		{
			io_context_t *io_ctx = tr_base::io_context_ptr(ctx);
			evio_t *ev = io_ctx->event();

			// stop actvity cb
			tr_activity::deinit(ctx);

			// there can be a situation
			//  where the watcher (withing the connection) has not been started,
			//  but has ev_feed_event() (via activate_context()) called on it
			// So because the object using this iomachine assumes it can free the event
			//  after calling release_context(), it will do so.
			//  but since there is a pending still -> it will be executed by libev
			//  and access freed memory.
			//
			//  this call will stop the watcher regardless of being active and clear all pending events too
			ev_io_stop(tr_base::ev_loop(ctx), ev);

			// just for good measure
			ev_io_set(ev, io_ctx->fd(), EV_NONE);
		}

		static void activate_context(context_t *ctx, int io_requested_ops)
		{
			ev_feed_event(tr_base::ev_loop(ctx), tr_base::io_context_ptr(ctx)->event(), io_requested_ops);
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

	public:

		// TODO: this is read-priority one
		//  can make many of these easily, but will need some duplicated code
		//  (or macros)
		static void run_loop(context_t *ctx, int const io_requested_ops = EV_READ | EV_WRITE)
		{
			// operations we're allowed to execute in this run
			int const io_allowed_ops = tr_allowed_ops::get(ctx);

			// the operations to be executed this run
			int io_executed_ops = (io_allowed_ops & io_requested_ops);
			if (EV_CUSTOM != io_requested_ops)
				io_executed_ops |= (EV_WRITE & io_allowed_ops);

			// operations we want to wait for, modified while looping
			int io_wait_ops = EV_NONE;

			// operations we have actually executed, for idle notification
			int io_activity_ops = EV_NONE;

			// we need this everywhere and it's cached for an iteration
			//  might be a wrong thing to do tho, time will tell
			io_context_t *io_ctx = tr_base::io_context_ptr(ctx);

			if (tr_log::is_allowed(ctx))
			{
				tr_log::write(ctx, line_mode::single, "{0}; fd: {1}; aop: 0x{2}, rop: 0x{3}, eop: 0x{4}"
						, __func__, io_ctx->fd()
						, meow::format::as_hex(io_allowed_ops)
						, meow::format::as_hex(io_requested_ops)
						, meow::format::as_hex(io_executed_ops)
						);
			}

			for (int io_current_ops = io_executed_ops; EV_NONE != io_current_ops; /**/)
			{
				if (tr_log::is_allowed(ctx))
				{
					tr_log::write(ctx, line_mode::single, "{0}; io_current_ops = 0x{1}, ev_ops: 0x{2}"
							, __func__
							, meow::format::as_hex(io_current_ops)
							, meow::format::as_hex(io_ctx->event()->events)
							);
				}

				if (bitmask_test(io_current_ops, EV_CUSTOM))
				{
					custom_op_status_t c_status = tr_custom_op::custom_operation(ctx);
					bitmask_clear(io_current_ops, EV_CUSTOM);

					io_activity_ops |= EV_CUSTOM;

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
					if (!tr_read_precheck::has_data_or_error(ctx, io_ctx))
					{
						if (tr_log::is_allowed(ctx))
						{
							tr_log::write(ctx, line_mode::single
									, "{0}; tr_read_precheck::has_data_or_error(): no available data on socket"
									, __func__
									);
						}
						bitmask_clear(io_current_ops, EV_READ);
						bitmask_set(io_wait_ops, EV_READ);
						break;
					}

					// now get a buffer
					meow::buffer_ref buf_to = tr_read::get_buffer(ctx);

					// don't read anymore till re-entering this function
					if (buf_to.empty())
					{
						if (tr_log::is_allowed(ctx))
						{
							tr_log::write(ctx, line_mode::single
									, "{0}; empty buffer from tr_read::get_buffer()"
									, __func__
									);
						}
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

						// we read nothing and are going to just wait for more
						// -> just say bye
						if (!r.filled_len)
							break;
					}

					// can consume buffer now, it's not empty as well
					meow::buffer_ref filled_buf(buf_to.begin(), r.filled_len);
					rd_consume_status_t c_status = tr_read::consume_buffer(ctx, filled_buf, r.status);

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

					io_activity_ops |= EV_READ;
				}

				if (tr_custom_op::requires_custom_op(ctx))
				{
					bitmask_set(io_current_ops, EV_CUSTOM);
					continue;
				}

				while (bitmask_test(io_current_ops, EV_WRITE))
				{
					meow::buffer_ref buf = tr_write::get_buffer(ctx);

					if (buf.empty())
					{
						if (tr_log::is_allowed(ctx))
						{
							tr_log::write(ctx, line_mode::single
									, "{0}; empty buffer from tr_write::get_buffer()"
									, __func__
									);
						}
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
					wr_complete_status_t c_status = tr_write::write_complete(ctx, written_buf, r.status);

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

					io_activity_ops |= EV_WRITE;
				}

				if (tr_custom_op::requires_custom_op(ctx))
				{
					bitmask_set(io_current_ops, EV_CUSTOM);
					continue;
				}

			} // while

			// do activity notification
			//  before doing any waiting
			if (io_activity_ops)
			{
				if (tr_log::is_allowed(ctx))
					tr_log::write(ctx, line_mode::single, "{0}; fd: {1}, activity: {2}", __func__, io_ctx->fd(), io_activity_ops);

				tr_activity::on_activity(ctx, io_activity_ops);
			}

			// now adjust what we're waiting for based on what
			//  we were waiting for and on what we've been executing in this loop

			// io_wait_ops contains bits set for events we want to wait for
			//  bit limited only to the (io_requested_ops & io_allowed_ops)
			//  so we need to only change those bits in new value for wait

			int new_wait_ops = io_ctx->event()->events; 	// the initial mask is unchanged
			bitmask_clear(new_wait_ops, io_executed_ops); 	// clear out masked space
			bitmask_set(new_wait_ops, io_wait_ops); 		// set the new event bits

			if (tr_log::is_allowed(ctx))
			{
				tr_log::write(    ctx, line_mode::single
								, "{0}; fd: {1}; loop end; curr_ev_ops: 0x{2}, new_wait_ops = 0x{3}"
								, __func__
								, io_ctx->fd()
								, meow::format::as_hex(io_ctx->event()->events)
								, meow::format::as_hex(new_wait_ops)
								);
			}

			if (io_ctx->event()->events != new_wait_ops)
			{
				evloop_t *loop = tr_base::ev_loop(ctx);

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
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__IO_MACHINE_HPP_

