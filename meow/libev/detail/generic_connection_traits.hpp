////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_
#define MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_

#include <sys/uio.h> // writev()

#include <boost/foreach.hpp>

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

	template<class ContextT, class Traits>
	struct generic_connection_traits : public Traits
	{
		typedef ContextT context_t;

		struct log_writer__default
		{
			static bool is_allowed(context_t *ctx) { return false; }
			static void write(context_t *ctx, meow::line_mode_t, str_ref const&) {}
		};

		MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, log_writer, log_writer__default);

		struct base
		{
			static evloop_t* ev_loop(context_t *ctx)
			{
				return ctx->loop_;
			}

			static io_context_t* io_context_ptr(context_t *ctx)
			{
				return &ctx->io_ctx_;
			}

			static context_t* context_from_io(libev::io_context_t *io_ctx)
			{
				return MEOW_SELF_FROM_MEMBER(context_t, io_ctx_, io_ctx);
			}

			static int io_allowed_ops(context_t *ctx)
			{
				return EV_READ | EV_WRITE | EV_CUSTOM;
			}
		};

		struct write
		{
			#define IO_LOG_WRITE(ctx, lmode, fmt, ...) 				\
				do { if (log_writer::is_allowed(ctx))				\
					log_writer::write(ctx, lmode, meow::format::fmt_tmp<1024>(fmt, ##__VA_ARGS__));	\
				} while(0)											\
			/**/

			static bool get_buffer(context_t *ctx, buffer_ref *result)
			{
				buffer_chain_t& wchain = ctx->wchain_;

				if (wchain.empty())
				{
					*result = buffer_ref();
				}
				else
				{
					*result = wchain.front()->used_part();
				}

				return true;
			}

			static wr_complete_status_t write_complete(context_t *ctx, buffer_ref written_br, write_status_t w_status)
			{
				if (write_status::error == w_status)
				{
					ctx->cb_write_closed(io_close_report(io_close_reason::io_error, errno));
					return wr_complete_status::closed;
				}
				if (write_status::closed == w_status)
				{
					ctx->cb_write_closed(io_close_report(io_close_reason::peer_close));
					return wr_complete_status::closed;
				}

				buffer_chain_t& wchain = ctx->wchain_;

				// check if we did actualy write something
				if (!written_br.empty())
				{
					buffer_t *b = wchain.front();
					b->advance_first(written_br.size());

					if (0 == b->used_size())
						wchain.pop_front();
				}
				else
				{
					BOOST_ASSERT(write_status::again == w_status);
				}

				// NOTE: don't need to check for close here
				//  the custom_op checker in io_machine takes care of it

				return (wchain.empty())
						? wr_complete_status::finished
						: wr_complete_status::more
						;
			}

			static wr_complete_status_t writev_bufs(context_t *ctx)
			{
				buffer_chain_t& wchain = ctx->wchain_;

				static size_t const writev_max_bufs = 8;
				struct iovec iov[writev_max_bufs];

				struct iovec *bufs = iov;
				size_t n_bufs = 0;

				typedef buffer_chain_t::iterator b_iter_t;
				b_iter_t b_i = wchain.begin();
				b_iter_t end_i = wchain.end();

				io_context_t *io_ctx = &ctx->io_ctx_;

				while (n_bufs > 0 || !wchain.empty())
				{
					// move the data to the beginning
					if (bufs != iov)
					{
						std::memmove(iov, bufs, n_bufs * sizeof(iov[0]));
						bufs = iov;
					}

					// fill up iovec as much as we can from the last known position
					if (n_bufs < writev_max_bufs)
					{
						while (b_i != end_i)
						{
							buffer_t *b = *b_i;

							bufs[n_bufs].iov_base = b->first;
							bufs[n_bufs].iov_len = b->used_size();

							++b_i;
							++n_bufs;

							if (n_bufs >= writev_max_bufs)
								break;
						}
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
							BOOST_ASSERT(n_bufs > 0);
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

		struct custom_op
		{
			static bool requires_custom_op(context_t *ctx)
			{
				if (!ctx->flags->is_closing)
					return false;

				if (ctx->flags->write_before_close)
					return ctx->wchain_.empty();

				return true;
			}

			static custom_op_status_t custom_operation(context_t *ctx)
			{
				BOOST_ASSERT(ctx->flags->is_closing);

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

