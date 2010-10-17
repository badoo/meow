////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_
#define MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_

#include <meow/bitfield_union.hpp>
#include <meow/unix/fcntl.hpp>

#include "io_machine.hpp"
#include "detail/generic_connection_traits.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
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

			while (!ctx->cb_is_closing_immediately() && !ctx->cb_is_closing_after_write())
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

	template<
		  class Traits
		, class Interface = mmc_connection_t
	>
	struct mmc_connection_impl_t 
		: public Interface
		, public mmc_connection_traits_read<Traits>::context_t
	{
		typedef mmc_connection_impl_t 			self_t;
		typedef typename Interface::events_t 	events_t;
		typedef generic_connection_traits_base<self_t> base_traits_t;

		typedef libev::io_machine_t<
					  self_t
					, base_traits_t
					, mmc_connection_traits_read<Traits>
					, generic_connection_traits_write<base_traits_t>
					, generic_connection_traits_custom_op
				> iomachine_t;

	public: // traits need access to this stuff

		evloop_t 		*loop_;
		io_context_t 	io_;

		buffer_chain_t 	wchain_;

		struct close_data_t
		{
			bool after_write : 1;
			bool immediately : 1;
		};
		typedef meow::bitfield_union<close_data_t> close_t;
		close_t 		close_;

	private:
		events_t 		*ev_;

	public:

		mmc_connection_impl_t(evloop_t *loop, int fd, events_t *ev)
			: loop_(loop)
			, io_(fd)
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

		bool cb_is_closing() const { return close_ != 0; }
		bool cb_is_closing_after_write() const { return close_->after_write != 0; }
		bool cb_is_closing_immediately() const { return close_->immediately != 0; }

		bool cb_log_is_allowed() { return Traits::log_is_allowed(this); }
		void cb_log_debug(line_mode_t lmode, str_ref msg) { Traits::log_message(this, lmode, msg); }

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
			this->close_->after_write = 1;
			iomachine_t::w_activate(this);
		}

		virtual void close_immediately()
		{
			this->close_->immediately = 1;
			iomachine_t::custom_activate(this);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_

