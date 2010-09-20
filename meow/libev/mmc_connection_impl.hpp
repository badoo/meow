////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_
#define MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_

#include <meow/buffer.hpp>
#include <meow/buffer_chain.hpp>
#include <meow/format/format.hpp>
#include <meow/format/format_tmp.hpp>
#include <meow/str_ref_algo.hpp> 		// strstr_ex

#include <meow/unix/fcntl.hpp> 			// os_unix::nonblocking
#include <meow/utility/offsetof.hpp> 	// for MEOW_SELF_FROM_MEMBER

#include "libev.hpp"
#include "io_machine.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ContextT>
	struct mmc_connection_traits_base
	{
		typedef ContextT context_t;

		static libev::evloop_t* ev_loop(context_t *ctx) { return ctx->loop_; }
		static libev::io_context_t* io_context_ptr(context_t *ctx) { return &ctx->io_; }

		static context_t* context_from_io(libev::io_context_t *io_ctx)
		{
			return MEOW_SELF_FROM_MEMBER(context_t, io_, io_ctx);
		}

		static int io_allowed_ops(context_t *ctx)
		{
			return EV_READ | EV_WRITE | EV_CUSTOM;
		}

		#define DEFINE_CONNECTION_TRAITS_FMT_FUNCTION(z, n, d) 							\
			template<class F FMT_TEMPLATE_PARAMS(n)> 									\
			static void log_debug(context_t *ctx, line_mode_t lmode, F const& fmt FMT_DEF_PARAMS(n)) 		\
			{ 																			\
				ctx->cb_log_debug(lmode, format::fmt_tmp<1024>(fmt FMT_CALL_SITE_ARGS(n))); 	\
			} 																			\
		/**/

		BOOST_PP_REPEAT(32, DEFINE_CONNECTION_TRAITS_FMT_FUNCTION, _);
	};

	struct mmc_connection_traits_write
	{
		template<class ContextT>
		static buffer_ref write_get_buffer(ContextT *ctx)
		{
			buffer_chain_t& wchain = ctx->wchain_ref();

			if (wchain.empty())
				return buffer_ref();

			return wchain.front()->used_part();
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

			if (wchain.empty())
			{
				if (ctx->close_after_write_)
				{
					ctx->cb_write_closed(io_close_report(io_close_reason::write_close));
					return wr_complete_status::closed;
				}
				return wr_complete_status::finished;
			}

			return wr_complete_status::more;
		}
	};

	struct mmc_connection_traits_custom_op
	{
		template<class ContextT>
		static bool requires_custom_op(ContextT *ctx)
		{
			return ctx->is_closing_;
		}

		template<class ContextT>
		static custom_op_status_t custom_operation(ContextT *ctx)
		{
			BOOST_ASSERT(ctx->is_closing_);

			ctx->cb_custom_closed(io_close_report(io_close_reason::custom_close));
			return custom_op_status::closed;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

	struct mmc_connection_traits_example
	{
		static size_t const max_message_length = 1024;

		template<class ContextT>
		static str_ref read_fetch_message(ContextT*, str_ref buffer_s)
		{
		}
	};

#endif

	template<class Traits>
	struct mmc_connection_traits_read
	{
		typedef mmc_connection_traits_read 	self_t;
		typedef str_ref 					message_t;

		struct context_t
		{
			buffer_move_ptr r_buf;

			context_t()
				: r_buf(new buffer_t(Traits::max_message_length))
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
		static buffer_ref read_get_buffer(ContextT *ctx)
		{
			return ctx->r_buf->free_part();
		}

		template<class ContextT>
		static rd_consume_status_t read_consume_buffer(ContextT *ctx, buffer_ref read_part, read_status_t r_status)
		{
			// now we might have our connection dead already
			if (read_status::error == r_status)
			{
				ctx->cb_read_closed(io_close_report(io_close_reason::io_error, errno));
				return rd_consume_status::closed;
			}

			if (read_status::closed == r_status)
			{
				ctx->cb_read_closed(io_close_report(io_close_reason::peer_close));
				return rd_consume_status::closed;
			}

			buffer_move_ptr& b = ctx->r_buf;
			b->advance_last(read_part.size());

			while (!ctx->is_closing_ && !ctx->close_after_write_)
			{
				message_t const message_s = Traits::read_fetch_message(ctx, b->used_part());
				if (!message_s)
				{
					if (b->full())
					{
						// try recover by moving the data around
						self_t::buffer_move_used_part_to_front(*b);

						// if it's still full -> we have to bail
						if (b->full())
						{
							ctx->cb_reader_error("header is too long");
							return rd_consume_status::finished;
						}
					}

					return rd_consume_status::more;
				}
				else
				{
					ctx->cb_reader_message(message_s);

					// move to the remainder of the data
					//  that can be the next request
					b->advance_first(message_s.size());
				}
			}

			// if we got here, it means we're closing!
			return rd_consume_status::closed;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits>
	struct mmc_connection_impl_t 
		: public mmc_connection_t
		, public mmc_connection_traits_read<Traits>::context_t
	{
		typedef mmc_connection_impl_t 	self_t;
		typedef mmc_connection_traits_base<self_t> base_traits_t;

		typedef libev::io_machine_t<
					  self_t
					, base_traits_t
					, mmc_connection_traits_read<Traits>
					, mmc_connection_traits_write
					, mmc_connection_traits_custom_op
				> iomachine_t;

	public: // traits need access to this stuff

		evloop_t 		*loop_;
		io_context_t 	io_;

		buffer_chain_t 	wchain_;

		bool 			close_after_write_ : 1;
		bool 			is_closing_ : 1;

	private:
		events_t 		*ev_;

	public:

		mmc_connection_impl_t(evloop_t *loop, int fd, events_t *ev)
			: loop_(loop)
			, io_(fd)
			, close_after_write_(0)
			, is_closing_(0)
			, ev_(ev)
		{
			os_unix::nonblocking(fd);
			iomachine_t::prepare_context(this);
		}

		~mmc_connection_impl_t()
		{
			iomachine_t::release_context(this);
		}

	public: // public ops for the brave, if you can cast to this type

		buffer_chain_t& wchain_ref()
		{
			return wchain_;
		}

	public: // callbacks

		void cb_read_closed(io_close_report_t const& r)	{ ev_->on_closed(this, r); }
		void cb_write_closed(io_close_report_t const& r) { ev_->on_closed(this, r); }
		void cb_custom_closed(io_close_report_t const& r) { ev_->on_closed(this, r); }

		void cb_reader_message(str_ref m) { ev_->on_message(this, m); }
		void cb_reader_error(str_ref m) { ev_->on_reader_error(this, m); }

		void cb_log_debug(line_mode_t lmode, str_ref msg) { Traits::log_debug(this, lmode, msg); }

	public:

		virtual int fd() const { return io_.fd(); }
		virtual void activate() { iomachine_t::rw_loop(this); }

	public:

		virtual void send(buffer_move_ptr buf)
		{
			wchain_.push_back(move(buf));
			iomachine_t::w_activate(this);
		}

		virtual void close_after_write()
		{
			this->close_after_write_ = 1;
			iomachine_t::w_activate(this);
		}

		virtual void close_immediately()
		{
			this->is_closing_ = 1;
			iomachine_t::custom_activate(this);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_

