////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__BIN_MSG_CONNECTION_IMPL_HPP_
#define MEOW_LIBEV__BIN_MSG_CONNECTION_IMPL_HPP_

#include <meow/bitfield_union.hpp>
#include <meow/unix/fcntl.hpp>

#include "io_context.hpp"
#include "io_machine.hpp"
#include "io_close_report.hpp"
#include "bin_msg_connection.hpp"
#include "detail/generic_connection_traits.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

	struct bin_msg_connection_traits_example
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
		static bool read_parse_header(ContextT*, header_t *result_h, str_ref header_s)
		{
		}

		// get's the full packet body length from the header
		static size_t read_header_get_body_length(header_t const&)
		{
		}
	};

#endif

	MEOW_DEFINE_SMART_ENUM(read_state, 
							((header, "header"))
							((body, "body"))
							);

	template<class Traits>
	struct bin_msg_connection_traits_read
	{
		typedef typename Traits::header_t header_t;

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
		static buffer_ref read_get_buffer(ContextT *ctx)
		{
			buffer_move_ptr& b = ctx->r_buf;

			if (!b)
				b.reset(new buffer_t(sizeof(header_t)));

			switch (ctx->r_state)
			{
				case read_state::header:
					return buffer_ref(b->begin(), b->begin() + sizeof(header_t));

				case read_state::body:
					return b->free_part();
			};

			BOOST_ASSERT(!"can't be reached");
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

			switch (ctx->r_state)
			{
				case read_state::header:
					// do we have full header buffered now?
					if (b->used_size() < sizeof(header_t))
						break;

					if (!Traits::read_parse_header(ctx, &ctx->r_header, b->used_part()))
						return rd_consume_status::closed;

					// buffer will have full data in it, including header
					//  which is already inside
					b->resize_to(b->size() + Traits::read_header_get_body_length(ctx->r_header));
					ctx->r_state = read_state::body;

					/*
					ff::fmt(stdout
							, "{0}; b: {{ size: {1}, used: {2}, free: {3} }\n"
							, __func__
							, b->size(), b->used_size(), b->free_size()
							);
					*/

					break;
					/* don't need a loop here to fetch extra messages
					 	as we're reading only sizeof(header) bytes first at start
					*/

				case read_state::body:
					// do we have full body in the buffer?
					if (b->used_size() < Traits::read_header_get_body_length(ctx->r_header) + sizeof(header_t))
						break;

					ctx->cb_read_buffer(ctx->r_header, move(b));
					break;
			}

			return rd_consume_status::more;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class Traits
		, class Interface = bin_msg_connection_t<typename Traits::header_t>
	>
	struct bin_msg_connection_impl_t 
		: public Interface
		, public bin_msg_connection_traits_read<Traits>::context_t
	{
		typedef bin_msg_connection_impl_t 				self_t;
		typedef generic_connection_traits_base<self_t> 	base_traits_t;

		typedef bin_msg_connection_traits_read<Traits> 	reader_traits_t;
		typedef typename reader_traits_t::header_t 		reader_header_t;
		typedef bin_msg_connection_t<reader_header_t> 	base_t;

		typedef libev::io_machine_t<
					  self_t
					, base_traits_t
					, reader_traits_t
					, generic_connection_traits_write<base_traits_t>
					, generic_connection_traits_custom_op
				> iomachine_t;

		typedef typename base_t::events_t events_t;

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

		bin_msg_connection_impl_t(evloop_t *loop, int fd, events_t *ev)
			: loop_(loop)
			, io_(fd)
			, ev_(ev)
		{
			os_unix::nonblocking(fd);
			iomachine_t::prepare_context(this);
		}

		~bin_msg_connection_impl_t()
		{
			iomachine_t::release_context(this);
		}

	public: // callbacks from traits

		void cb_read_closed(io_close_report_t const& r)	{ ev_->on_closed(this, r); }
		void cb_write_closed(io_close_report_t const& r) { ev_->on_closed(this, r); }
		void cb_custom_closed(io_close_report_t const& r) { ev_->on_closed(this, r); }

		bool cb_is_closing() const { return close_ != 0; }
		bool cb_is_closing_after_write() const { return close_->after_write != 0; }
		bool cb_is_closing_immediately() const { return close_->immediately != 0; }

		void cb_read_buffer(reader_header_t const& h, buffer_move_ptr body)
		{
			return ev_->on_read(this, h, move(body));
		}

		void cb_read_error(str_ref error_msg)
		{
			return ev_->on_read_error(this, error_msg);
		}

		void cb_log_debug(line_mode_t lmode, str_ref msg)
		{
			Traits::log_debug(this, lmode, msg);
		}

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
			this->close_->after_write = true;
			iomachine_t::w_activate(this);
		}

		virtual void close_immediately()
		{
			this->close_->immediately = true;
			iomachine_t::custom_activate(this);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__BIN_MSG_CONNECTION_IMPL_HPP_

