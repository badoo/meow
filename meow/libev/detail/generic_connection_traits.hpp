////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_
#define MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_

#include <sys/uio.h> // writev()

#include <meow/utility/offsetof.hpp> 	// for MEOW_SELF_FROM_MEMBER

#include <meow/buffer.hpp>
#include <meow/buffer_chain.hpp>

#include <meow/format/format.hpp> 		// FMT_TEMPLATE_PARAMS, etc.
#include <meow/format/format_tmp.hpp>

#include <meow/libev/libev.hpp>
#include <meow/libev/io_context.hpp>
#include <meow/libev/io_machine.hpp> 	// read/write statuses
#include <meow/libev/io_close_report.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	#define IO_LOG_WRITE(ctx, lmode, fmt, ...) 				\
		do { if (log_writer::is_allowed(ctx))				\
			log_writer::write(ctx, lmode, meow::format::fmt_tmp<1024>(fmt, ##__VA_ARGS__));	\
		} while(0)											\
	/**/

	template<class Traits>
	struct generic_connection_logging_traits
	{
		struct log_writer__default
		{
			template<class ContextT>
			static bool is_allowed(ContextT *ctx) { return false; }

			template<class ContextT>
			static void write(ContextT *ctx, meow::line_mode_t, str_ref const&) {}
		};

		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, log_writer, log_writer__default);
	};

	template<class Traits>
	struct generic_connection_traits__base
	{
		typedef Traits orig_traits;

		template<class ContextT>
		static evloop_t* ev_loop(ContextT *ctx)
		{
			return ctx->loop_;
		}

		template<class ContextT>
		static io_context_t* io_context_ptr(ContextT *ctx)
		{
			return &ctx->io_ctx_;
		}

		template<class ContextT>
		static int io_allowed_ops(ContextT *ctx)
		{
			return EV_READ | EV_WRITE | EV_CUSTOM;
		}
	};


	template<class BaseTraits>
	struct generic_connection_traits__write
	{
		typedef typename BaseTraits::orig_traits orig_traits;
		typedef typename generic_connection_logging_traits<orig_traits>::log_writer log_writer;

		template<class ContextT>
		static wr_complete_status_t writev_bufs(ContextT *ctx)
		{
			return writev_from_wchain(ctx, ctx->wchain_);
		}

		template<class ContextT>
		static wr_complete_status_t writev_from_wchain(ContextT *ctx, buffer_chain_t& wchain)
		{
			io_context_t *io_ctx = BaseTraits::io_context_ptr(ctx);

			IO_LOG_WRITE(ctx, line_mode::single, "{0}; ctx: {1}, wsz: {2}"
					, __func__, ctx, wchain.size() /* linear complexity, but usually very short */);

			if (wchain.empty())
				return wr_complete_status::finished;

			// check for just 1 buffer in the chain
			//  if that - use plain write() syscall
			if (wchain.end() == ++wchain.begin())
			{
				buffer_t *b = wchain.front();

				size_t offset = 0;
				size_t const total_size = b->used_size();

				while (offset < total_size)
				{
					size_t const wr_size = total_size - offset;

					IO_LOG_WRITE(ctx, line_mode::prefix
							, "::write({0}, {1} + {2}, {3} = {4} - {5}) = "
							, io_ctx->fd(), (void*)b->first, offset
							, wr_size, total_size, offset
							);

					ssize_t const n = ::write(io_ctx->fd(), b->first + offset, wr_size);

					if (-1 == n)
					{
						IO_LOG_WRITE(ctx, line_mode::suffix, "{0}, errno: {1} : {2}", n, errno, strerror(errno));

						if (EAGAIN == errno || EWOULDBLOCK == errno)
							break;

						ctx->cb_write_closed(io_close_report(io_close_reason::io_error, errno));
						return wr_complete_status::closed;
					}
					else
					{
						IO_LOG_WRITE(ctx, line_mode::suffix, "{0}", n);

						if (0 == n)
						{
							ctx->cb_write_closed(io_close_report(io_close_reason::peer_close));
							return wr_complete_status::closed;
						}

						offset += n;
					}
				}

				b->advance_first(offset);

				if (!b->empty())
					return wr_complete_status::more;

				wchain.pop_front();
				return wr_complete_status::finished;
			}

			static size_t const writev_max_bufs = 8;
			struct iovec iov[writev_max_bufs];

			struct iovec *bufs = iov;
			size_t n_bufs = 0;

			// this iterator is moved along the chain
			//  and always points to the first buffer we don't have in iov yet
			// NOTE: wchain is getting changed while writing, so care needs to be taken to not make iterator invalid
			buffer_chain_t::iterator b_i = wchain.begin();

			while (n_bufs > 0 || !wchain.empty())
			{
				// move the data to the beginning
				//  as we need to make some space at end to insert more data
				if (bufs != iov)
				{
					std::memmove(iov, bufs, n_bufs * sizeof(iov[0]));
					bufs = iov;
				}

				// fill up iovec as much as we can from the last known position
				while (n_bufs < writev_max_bufs && b_i != wchain.end())
				{
					buffer_t *b = *b_i;

					bufs[n_bufs].iov_base = b->first;
					bufs[n_bufs].iov_len = b->used_size();

					++b_i;
					++n_bufs;
				}

				IO_LOG_WRITE(ctx, line_mode::prefix, "::writev({0}, {1} : ", io_ctx->fd(), n_bufs);

				size_t total_len = 0;
				for (size_t i = 0; i < n_bufs; ++i)
				{
					struct iovec *v = bufs + i;
					total_len += v->iov_len;

					IO_LOG_WRITE(ctx, line_mode::middle, "{2}{{ {0}, {1} }"
							, v->iov_base, v->iov_len
							, ((i > 0) ? ", " : "")
							);
				}

				IO_LOG_WRITE(ctx, line_mode::middle, ", {0}) = ", total_len);
				ssize_t n = ::writev(io_ctx->fd(), bufs, n_bufs);
				if (n >= 0)
					IO_LOG_WRITE(ctx, line_mode::suffix, "{0}", n);
				else
					IO_LOG_WRITE(ctx, line_mode::suffix, "{0}; {1} - {2}", n, errno, strerror(errno));

				switch (n)
				{
				case -1:
					if (EAGAIN == errno || EWOULDBLOCK == errno)
					{
						return wr_complete_status::more;
					}
					else
					{
						ctx->cb_write_closed(io_close_report(io_close_reason::io_error, errno));
						return wr_complete_status::closed;
					}

				case 0:
					ctx->cb_write_closed(io_close_report(io_close_reason::peer_close));
					return wr_complete_status::closed;

				default:
					size_t len = n;
					while (len > 0 && len >= bufs->iov_len)
					{
						len -= bufs->iov_len;
						bufs++;

						// we assume that writev() never returns more than all the buffers
						//  could hold, so n_bufs can't underflow here
						assert(n_bufs > 0);
						n_bufs--;

						// buf fully written
						wchain.pop_front();
					}

					if (len > 0)
					{
						bufs->iov_base = (char*)bufs->iov_base + len;
						bufs->iov_len -= len;

						// partial write, adjust anyway, might not have the chance later
						buffer_t *b = wchain.front();
						b->advance_first(len);
					}
					break;
				} // switch
			} // while

			return wr_complete_status::finished;
		}
	};

	template<class Traits>
	struct generic_connection_traits : public Traits
	{
		typedef typename generic_connection_logging_traits<Traits>::log_writer log_writer;

		typedef generic_connection_traits__base<Traits>  base;
		typedef generic_connection_traits__write<base>   write;

		struct custom_op
		{
			template<class ContextT>
			static bool requires_custom_op(ContextT *ctx)
			{
				if (!ctx->flags->is_closing)
					return false;

				if (ctx->flags->write_before_close)
					return !ctx->has_buffers_to_send();

				return true;
			}

			template<class ContextT>
			static custom_op_status_t custom_operation(ContextT *ctx)
			{
				assert(ctx->flags->is_closing);

				io_close_reason_t close_reason =
					(ctx->flags->write_before_close)
						? io_close_reason::write_close
						: io_close_reason::custom_close
						;

				ctx->cb_custom_closed(io_close_report(close_reason));
				return custom_op_status::closed;
			}
		};

	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_

