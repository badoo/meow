////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_
#define MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_

#include <meow/utility/nested_name_alias.hpp>

#include <meow/libev/mmc_connection.hpp>
#include <meow/libev/detail/generic_connection_impl.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

	// mmc specific traits member
	struct mmc_connection_traits_example
	{
		/* whatever other stuff you need, like base, write, etc. */

		struct mmc_read
		{
			static size_t const max_message_length = 1024;

			template<class ContextT>
			static str_ref fetch_message(ContextT*, str_ref buffer_s) {}
		};
	};

#endif

	template<class Traits>
	struct mmc_connection_repack_traits : public Traits
	{
		typedef typename Traits::mmc_read tr_mmc_read;

		struct read
		{
			typedef str_ref message_t;

			struct context_t
			{
				buffer_move_ptr r_buf;

				context_t()
					: r_buf(new buffer_t(tr_mmc_read::max_message_length))
				{
				}
			};
		private:

			static buffer_t& buffer_move_used_part_to_front(buffer_t& buf)
			{
				if (buf.begin() != buf.first)
				{
					meow::str_ref const remainder_s = buf.used_part();
					std::memmove(buf.begin(), remainder_s.begin(), remainder_s.size());
					buf.reset_first(buf.begin());
					buf.reset_last(buf.begin() + remainder_s.size());
				}

				return buf;
			}

		public:

			template<class ContextT>
			static buffer_ref get_buffer(ContextT *ctx)
			{
				return ctx->r_buf->free_part();
			}

			template<class ContextT>
			static rd_consume_status_t consume_buffer(ContextT *ctx, buffer_ref read_part, read_status_t r_status)
			{
				// now we might have our connection dead already
				if (read_status::error == r_status)
				{
					MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, on_closed, io_close_report(io_close_reason::io_error, errno));
					return rd_consume_status::closed;
				}

				if (read_status::closed == r_status)
				{
					MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, on_closed, io_close_report(io_close_reason::peer_close));
					return rd_consume_status::closed;
				}

				buffer_move_ptr& b = ctx->r_buf;
				b->advance_last(read_part.size());

				while (!ctx->is_closing())
				{
					message_t const message_s = tr_mmc_read::fetch_message(ctx, b->used_part());
					if (!message_s)
					{
						if (b->full())
						{
							// try recover by moving the data around
							buffer_move_used_part_to_front(*b);

							// if it's still full -> we have to bail
							if (b->full())
							{
								MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, on_reader_error, "header is too long");

								b->clear();
								return rd_consume_status::more;
							}
						}

						return rd_consume_status::more;
					}
					else
					{
						MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, on_message, message_s);

						// move to the remainder of the data
						//  that can be the next request
						b->advance_first(message_s.size());
					}
				}

				// if we got here, it means we're closing!
				return rd_consume_status::closed;
			}
		};
	};

	DEFINE_CONNECTION_WRAPPER(mmc_connection_impl_t, mmc_connection_repack_traits, mmc_connection_t);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_

