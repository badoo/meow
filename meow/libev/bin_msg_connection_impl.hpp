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
			// the header struct that is in the beginning of every packet
			// NOTE: maybe should just expose length of the header as a separate parameter
			struct header_t
			{
				uint32_t data_length;

				header_t()
					: data_length(0)
				{
				}
			};

			// 
			// parses a header from the buffer that was read
			//  validates it's correctness
			//  and fills result_h data
			// returns
			//  true - if parsing and validation are fine,
			//  		result_h is filled with valid data
			//  		and we can continue reading the body
			//  false - parsing or validation failed
			//  		 probably some sort of ctx callback
			//  		 was invoked to report the failure
			//
			template<class ContextT>
			static bool parse_header(ContextT*, header_t *result_h, str_ref header_s)
			{
			}

			// get's the full packet body length from the header
			static size_t header_get_body_length(header_t const&)
			{
			}
		};
	};
#endif

	template<class Traits>
	struct bin_msg_connection_repack_traits : public Traits
	{
		struct read
		{
			typedef typename Traits::bin_msg_read tr;
			typedef typename tr::header_t 	header_t;

			MEOW_DEFINE_SMART_ENUM_STRUCT_T(read_state, 
											((header, "header"))
											((body, "body"))
											);

			struct context_t
			{
				read_state_t 	r_state;
				header_t 		r_header;
				buffer_move_ptr r_buf;

				context_t()
					: r_state(read_state::header)
				{
				}
			};

		public:

			template<class ContextT>
			static buffer_ref get_buffer(ContextT *ctx)
			{
				buffer_move_ptr& b = ctx->r_buf;

				if (!b)
					b.reset(new buffer_t(sizeof(header_t)));

				switch (ctx->r_state)
				{
					case read_state::header:
						BOOST_ASSERT(b->used_size() < sizeof(header_t));
						return buffer_ref(b->last, b->first + sizeof(header_t));

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

				switch (ctx->r_state)
				{
					case read_state::header:
						// do we have full header buffered now?
						if (b->used_size() < sizeof(header_t))
							break;

						if (!tr::parse_header(ctx, &ctx->r_header, b->used_part()))
						{
							// can do clear() as we always read maximum sizeof(header) first
							//  so there are no other requests in the buffer for sure
							b->clear();
							return rd_consume_status::closed;
						}

						// buffer will have full data in it, including header
						//  which is already inside
						b->resize_to(b->size() + tr::header_get_body_length(ctx->r_header));
						ctx->r_state = read_state::body;

						break; // got no body in the buffer yet anyway

					case read_state::body:
						// do we have full body in the buffer?
						if (b->used_size() < tr::header_get_body_length(ctx->r_header) + sizeof(header_t))
							break;

						MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, on_read, ctx->r_header, move(b));

						// go for the next request
						ctx->r_state = read_state::header;
						ctx->r_header = header_t();
						break;
				}

				return rd_consume_status::more;
			}
		};
	};

	MEOW_LIBEV_DEFINE_CONNECTION_WRAPPER(
						  bin_msg_connection_impl_t
						, bin_msg_connection_repack_traits
						, bin_msg_connection_t<typename Traits::bin_msg_read::header_t>
						);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__BIN_MSG_CONNECTION_IMPL_HPP_

