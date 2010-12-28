////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_HPP_
#define MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_HPP_

#include <boost/noncopyable.hpp>

#include <meow/buffer.hpp>
#include <meow/buffer_chain.hpp>
#include <meow/str_ref.hpp>

#include <meow/bitfield_union.hpp>
#include <meow/unix/fcntl.hpp>
#include <meow/utility/nested_name_alias.hpp>

#include <meow/libev/libev.hpp>
#include <meow/libev/io_close_report.hpp>
#include <meow/libev/detail/generic_connection_traits.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct generic_connection_close_data_t
	{
		bool after_write : 1;
		bool immediately : 1;
	};
	typedef meow::bitfield_union<generic_connection_close_data_t> close_flags_t;

	struct generic_connection_t
		: private boost::noncopyable
	{
		virtual ~generic_connection_t() {}

	public: // general info

		virtual int 		fd() const = 0;
		virtual evloop_t* 	loop() const = 0;

	public: // io

		virtual void r_loop() = 0;
		virtual void w_loop() = 0;
		virtual void rw_loop() = 0;

		virtual void activate(int revents) = 0;
		virtual void r_activate() = 0;
		virtual void w_activate() = 0;
		virtual void rw_activate() = 0;
		virtual void custom_activate() = 0;

		virtual void queue_buf(buffer_move_ptr) = 0;
		virtual void queue_chain(buffer_chain_t&) = 0;

		virtual void send(buffer_move_ptr) = 0;
		virtual void send_chain(buffer_chain_t&) = 0;

	public: // closing

		virtual bool 			is_closing() const = 0;
		virtual close_flags_t 	close_flags() const = 0;

		virtual void close_after_write() = 0;
		virtual void close_immediately() = 0;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Interface, class Traits>
	struct generic_connection_impl_t
		: public Interface
		, public Traits::read::context_t
	{
		typedef generic_connection_impl_t 		self_t;
		typedef generic_connection_impl_t 		base_t;
		typedef typename Interface::events_t 	events_t;

		struct traits_t
		{
			typedef generic_connection_traits_base<self_t> 		base;
			typedef typename Traits::read 						read;
			typedef generic_connection_traits_write<self_t> 	write;

			//MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(Traits, custom_op, generic_connection_traits_custom_op);
			typedef generic_connection_traits_custom_op 		custom_op;

			MEOW_DEFINE_NESTED_NAME_ALIAS_OR_VOID(Traits, allowed_ops);
			MEOW_DEFINE_NESTED_NAME_ALIAS_OR_VOID(Traits, read_precheck);
			MEOW_DEFINE_NESTED_NAME_ALIAS_OR_VOID(Traits, log_writer);
			MEOW_DEFINE_NESTED_NAME_ALIAS_OR_VOID(Traits, activity_tracker);
		};

		typedef libev::io_machine_t<self_t, traits_t> iomachine_t;

	public:
		// traits need access to this stuff
		//  not all of it, but to avoid padding between members, we just have them in one place

		events_t 		*ev_;
		evloop_t 		*loop_;
		io_context_t 	io_;

		buffer_chain_t 	wchain_;
		close_flags_t 	close_;

	public: // callbacks, for the traits

		events_t* ev() const { return ev_; }

		void cb_read_closed(io_close_report_t const& r)	{ ev_->on_closed(this, r); }
		void cb_write_closed(io_close_report_t const& r) { ev_->on_closed(this, r); }
		void cb_custom_closed(io_close_report_t const& r) { ev_->on_closed(this, r); }

	public:

		generic_connection_impl_t(evloop_t *loop, int fd, events_t *ev)
			: loop_(loop)
			, io_(fd)
			, ev_(ev)
		{
			os_unix::nonblocking(fd);
			iomachine_t::prepare_context(this);
		}

		~generic_connection_impl_t()
		{
			iomachine_t::release_context(this);
		}

	public:

		virtual int fd() const { return io_.fd(); }
		virtual evloop_t* loop() const { return loop_; }

	public:

		virtual void r_loop() { iomachine_t::r_loop(this); }
		virtual void w_loop() { iomachine_t::w_loop(this); }
		virtual void rw_loop() { iomachine_t::rw_loop(this); }

		virtual void activate(int revents) { iomachine_t::activate_context(this, revents); }
		virtual void r_activate() { iomachine_t::r_activate(this); }
		virtual void w_activate() { iomachine_t::w_activate(this); }
		virtual void rw_activate() { iomachine_t::rw_activate(this); }
		virtual void custom_activate() { iomachine_t::custom_activate(this); }

	public:

		virtual void queue_buf(buffer_move_ptr buf)
		{
			wchain_.push_back(move(buf));
		}

		virtual void queue_chain(buffer_chain_t& chain)
		{
			wchain_.append_chain(chain);
		}

		virtual void send(buffer_move_ptr buf)
		{
			this->queue_buf(move(buf));
			this->w_activate();
		}

		virtual void send_chain(buffer_chain_t& chain)
		{
			this->queue_chain(chain);
			this->w_activate();
		}

	public: // closing

		virtual bool is_closing() const { return 0 != close_; }
		virtual close_flags_t close_flags() const { return close_; }

		virtual void close_after_write()
		{
			close_->after_write = 1;
			this->w_activate();
		}

		virtual void close_immediately()
		{
			close_->immediately = 1;
			this->custom_activate();
		}
	};

#define MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, cb_name, args...) \
	do { ctx->ev()->cb_name(ctx, args); } while(0)

////////////////////////////////////////////////////////////////////////////////////////////////

	struct mmc_connection_t : public generic_connection_t
	{
		virtual ~mmc_connection_t() {}

		struct events_t
		{
			virtual ~events_t() {}

			virtual void on_message(mmc_connection_t*, str_ref) = 0;
			virtual void on_reader_error(mmc_connection_t*, str_ref) = 0;
			virtual void on_closed(mmc_connection_t*, io_close_report_t const&) = 0;
		};
	};

	template<class Traits>
	struct mmc_connection_repack_traits : public Traits
	{
		typedef typename Traits::mmc_read 			tr_mmc_read;

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

#define DEFINE_CONNECTION_WRAPPER(name, traits, def_interface) 				\
	template<class Traits, class InterfaceT = def_interface >				\
	struct name 															\
		: public generic_connection_impl_t<InterfaceT, traits<Traits> >		\
	{																		\
		name(evloop_t *loop, int fd, typename InterfaceT::events_t *ev)		\
			: name::base_t(loop, fd, ev)									\
		{																	\
		}																	\
	};																		\
/**/

	DEFINE_CONNECTION_WRAPPER(mmc_connection_impl_t, mmc_connection_repack_traits, mmc_connection_t);

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class HeaderT>
	struct bin_msg_connection_t : public generic_connection_t
	{
		typedef bin_msg_connection_t 	self_t;
		typedef HeaderT 				header_t;

		struct events_t
		{
			virtual ~events_t() {}

			virtual void on_read(bin_msg_connection_t*, header_t const&, buffer_move_ptr) = 0;
			virtual void on_read_error(bin_msg_connection_t*, str_ref error_msg) = 0;
			virtual void on_closed(bin_msg_connection_t*, io_close_report_t const&) = 0;
		};
	};

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

	DEFINE_CONNECTION_WRAPPER(
						  bin_msg_connection_impl_t
						, bin_msg_connection_repack_traits
						, bin_msg_connection_t<typename Traits::bin_msg_read::header_t>
						);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_HPP_

