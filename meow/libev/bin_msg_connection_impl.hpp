////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__BIN_MSG_CONNECTION_IMPL_HPP_
#define MEOW_LIBEV__BIN_MSG_CONNECTION_IMPL_HPP_

#include <meow/smart_enum.hpp>
#include <meow/utility/nested_name_alias.hpp>

#include <meow/libev/bin_msg_connection.hpp>
#include <meow/libev/detail/generic_connection_impl.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

	struct bin_msg_connection_traits_example
	{
		struct bin_msg_read
		{
			static size_t const header_size = /* implementation-defined */;
		};
	};
#endif

	template<class Traits>
	struct bin_msg_connection_repack_traits : public Traits
	{
		struct read
		{
			typedef typename Traits::bin_msg_read tr;

			MEOW_DEFINE_SMART_ENUM_STRUCT_T(read_state, 
											((header, "header"))
											((body, "body"))
											);

			struct context_t
			{
				read_state_t 	r_state;
				buffer_move_ptr r_buf;

				context_t()
				{
					this->r_reset_for_next_request();
				}

				void r_reset_for_next_request()
				{
					r_state = read_state::header;
					r_buf.reset();
				}
			};

		public:

			template<class ContextT>
			static buffer_ref get_buffer(ContextT *ctx)
			{
				buffer_move_ptr& b = ctx->r_buf;

				if (!b)
					b.reset(new buffer_t(tr::header_size));

				switch (ctx->r_state)
				{
					case read_state::header:
						BOOST_ASSERT(b->used_size() <= tr::header_size);
						return buffer_ref(b->last, b->first + tr::header_size);

					case read_state::body:
						return b->free_part();
				};

				BOOST_ASSERT(!"can't be reached");
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

				switch (ctx->r_state) {
					case read_state::header:
					{
						// do we have full header buffered now?
						if (b->used_size() < tr::header_size)
							break;

						ssize_t const body_length = MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, on_header, b->used_part());

						// error parsing header, length unknown, assume close
						if (body_length < 0)
							return rd_consume_status::closed;

						if (body_length > 0)
							b->resize_to(b->size() + body_length);
					}

					/* fall through, can have body of size == 0 */

					case read_state::body:

						ctx->r_state = read_state::body;

						// reading till buffer fills up, as we resized it to the required target size
						if (!b->full())
							break;

						MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, on_message, move(b));

						ctx->r_reset_for_next_request();
						break;
				}

				return rd_consume_status::more;
			}
		};
	};

	MEOW_LIBEV_DEFINE_CONNECTION_WRAPPER(
						  bin_msg_connection_impl_t
						, bin_msg_connection_repack_traits
						, bin_msg_connection_t
						);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__BIN_MSG_CONNECTION_IMPL_HPP_

