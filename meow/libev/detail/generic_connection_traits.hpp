////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_
#define MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_

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

	template<class ContextT>
	struct generic_connection_traits_base
	{
		typedef ContextT context_t;

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

	template<class BaseTraits>
	struct generic_connection_traits_write
	{

		template<class ContextT>
		static buffer_ref get_buffer(ContextT *ctx)
		{
			buffer_chain_t& wchain = ctx->wchain_;

			// skip empy buffers
			// if the current buffer is empty
			//  (this can only happen if we someow had empty buffer in the chain)
			//  -> just drop it and try continue
			while (!wchain.empty())
			{
				buffer_ref r = wchain.front()->used_part();

				if (!r.empty())
					return r;

				wchain.pop_front();
			}

			return buffer_ref();
		}

		template<class ContextT>
		static wr_complete_status_t write_complete(ContextT *ctx, buffer_ref written_br, write_status_t w_status)
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
	};

	struct generic_connection_traits_custom_op
	{
		template<class ContextT>
		static bool requires_custom_op(ContextT *ctx)
		{
			if (!ctx->flags->is_closing)
				return false;

			if (ctx->flags->write_before_close)
				return ctx->wchain_.empty();

			return true;
		}

		template<class ContextT>
		static custom_op_status_t custom_operation(ContextT *ctx)
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

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_TRAITS_HPP_

