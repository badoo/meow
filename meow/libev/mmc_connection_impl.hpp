////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_
#define MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_

#include <meow/utility/nested_name_alias.hpp>

#include <meow/libev/mmc_connection.hpp>
#include <meow/libev/detail/dynamic_connection.hpp> // dynamic_reader_events_t
#include <meow/libev/detail/generic_connection_impl.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#if 0 && MMC_READER_OPERATIONS_TRAITS_DEFINITION_EXAMPLE

	struct reader_traits
	{
		struct ctx_info
		{
			template<class ConnectionT>
			static mmc_reader_ctx_t* get_context(ConnectionT *c)
			{
				mmc_reader_t *reader = static_cast<mmc_reader_t*>(c->ev());
				return reader;
			}

			// return whatever type you want, conforming to the duck_typed interface
			template<class ConnectionT>
			static events_t* get_events(ConnectionT *c)
			{
				mmc_reader_t *reader = static_cast<mmc_reader_t*>(c->ev());
				return reader->events;
			}
		};

		struct mmc_read
		{
			// returns
			// found: behind-the-end pointer for the the message fetched
			// not_found: NULL
			template<class ConnectionT>
			static char const* fetch_message(ConnectionT*, str_ref const& b)
			{
				char const *n = (char const*)std::memchr(b.begin(), '\n', b.size());
				return (NULL == n)
						? NULL
						: ++n
						;
			}

			template<class ConnectionT>
			static size_t max_message_length(ConnectionT *c)
			{
				return c->module->max_message_length;
			}
		};
	};

#endif

	// default and minimal reader context
	struct mmc_reader_ctx_t
	{
		buffer_move_ptr r_buf;
	};

	template<class Traits>
	struct mmc_reader_operations
	{
		typedef mmc_reader_operations  self_t;
		typedef Traits                 traits;

		typedef typename traits::ctx_info   tr_ctx_info;
		typedef typename traits::mmc_read   tr_mmc_read;

		typedef libev::read_status          read_status;
		typedef libev::read_status_t        read_status_t;
		typedef libev::rd_consume_status    rd_consume_status;
		typedef libev::rd_consume_status_t  rd_consume_status_t;
		typedef libev::io_close_report_t    io_close_report_t;

		template<class ConnectionT>
		static buffer_ref get_buffer(ConnectionT *c)
		{
			buffer_move_ptr& b = tr_ctx_info::get_context(c)->r_buf;

			if (!b)
				b.reset(new buffer_t(tr_mmc_read::max_message_length(c)));

			return b->free_part();
		}

		template<class ConnectionT>
		static rd_consume_status_t consume_buffer(ConnectionT* c, buffer_ref read_part, read_status_t r_status)
		{
			// now we might have our connection dead already
			if (read_status::error == r_status)
			{
				tr_ctx_info::get_events(c)->on_closed(c, io_close_report_t(libev::io_close_reason::io_error, errno));
				return rd_consume_status::closed;
			}

			if (read_status::closed == r_status)
			{
				tr_ctx_info::get_events(c)->on_closed(c, io_close_report_t(libev::io_close_reason::peer_close));
				return rd_consume_status::closed;
			}

			return self_t::read_process_buffer_data(c, read_part, (read_status::closed == r_status));
		}

		template<class ConnectionT>
		static rd_consume_status_t read_process_buffer_data(ConnectionT *c, buffer_ref read_part, bool is_closed)
		{
			buffer_move_ptr& b = tr_ctx_info::get_context(c)->r_buf;
			b->advance_last(read_part.size());

			while (!c->is_closing())
			{
				char const *found_e = tr_mmc_read::fetch_message(c, b->used_part());
				if (NULL == found_e)
				{
					if (b->full())
					{
						// try recover by moving the data around
						buffer_move_used_part_to_front(*b);

						// if it's still full -> we have to bail
						if (b->full())
						{
							b->clear();

							tr_ctx_info::get_events(c)->on_error(c, ref_lit("message is too long"));
							return rd_consume_status::loop_break;
						}
					}

					return rd_consume_status::more;
				}
				else
				{
					str_ref const message_s = str_ref(b->first, found_e);

					// move to the remainder of the data
					//  that can be the next request
					b->advance_first(message_s.size());

					// if the buffer is now empty
					//  we can just clear the buffer preemptively
					//  to avoid data moves in the future
					if (b->empty())
						b->clear();

					// this needs to be the last line, so that we go check for close after
					//  buffer->first/last have been updated, but data is intact and we have our message fetched
					bool const can_loop_more = tr_ctx_info::get_events(c)->on_message(c, message_s);
					if (!can_loop_more)
						return rd_consume_status::more;
				}
			}

			return rd_consume_status::closed;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits /* the client supplied traits here */ >
	struct mmc_connection_repack_traits : public Traits
	{
		// internal implementation detail,
		//  customising the behaviour of mmc_reader_operations<>
		//  for use with connection built-in reader
		struct mmc_reader_operations_traits__
		{
			struct ctx_info
			{
				template<class ConnectionT>
				static ConnectionT* get_context(ConnectionT *c)
				{
					return c;
				}

				template<class ConnectionT>
				static typename ConnectionT::events_t* get_events(ConnectionT *c)
				{
					return c->ev();
				}
			};

			struct mmc_read
			{
				template<class ConnectionT>
				static char const* fetch_message(ConnectionT *c, str_ref const& b)
				{
					return Traits::mmc_read::fetch_message(c, b);
				}

				template<class ConnectionT>
				static size_t max_message_length(ConnectionT *c)
				{
					return Traits::mmc_read::max_message_length(c);
				}
			};
		};

		struct read
		{
			typedef mmc_reader_ctx_t context_t; // needed by the generic_connection_impl_t

			typedef mmc_reader_operations<mmc_reader_operations_traits__> reader_traits;

			template<class ConnectionT>
			static buffer_ref get_buffer(ConnectionT *c)
			{
				return reader_traits::get_buffer(c);
			}

			template<class ConnectionT>
			static rd_consume_status_t consume_buffer(ConnectionT *c, buffer_ref read_part, read_status_t r_status)
			{
				return reader_traits::consume_buffer(c, read_part, r_status);
			}
		};
	};

	MEOW_LIBEV_DEFINE_CONNECTION_WRAPPER(mmc_connection_impl_t, mmc_connection_repack_traits, mmc_connection_t);

////////////////////////////////////////////////////////////////////////////////////////////////
// dynamic reader

#if 0 && MMC_READER_TRAITS_DEFINITION_EXAMPLE

	struct traits
	{
		// reader will inherit from this object, make sure it's default-constructible
		struct context_t
		{
			size_t read_buffer_size;
			context_t() : read_buffer_size(1024) {}
		};

		// fetch the message from buffer
		static char const* fetch_message(str_ref const& b)
		{
			return mmc_reader_fetch_message(b);
		}

		// return the maximum allowed message size that the reader will support
		//  c: the pointer to actual connection object
		//  reader: the pointer to the reader (that inherits from context_t above)
		//   you inject these traits into
		template<class ConnectionT, class ReaderT>
		static size_t max_message_length(ConnectionT *c, ReaderT *reader)
		{
			return reader->read_buffer_size;
		}
	};

#endif

	inline char const* mmc_reader_fetch_message(str_ref const& b)
	{
		char const *n = (char const*)std::memchr(b.begin(), '\n', b.size());
		return (NULL == n)
				? NULL
				: ++n
				;
	}

	template<class Derived>
	struct mmc_reader_t
		: public dynamic_reader_events_t
		, public mmc_reader_ctx_t
	{
		typedef mmc_reader_t         self_t;
		typedef mmc_reader_events_t  events_t;
		typedef self_t               base_reader_t; // for the Derived

	private:

		struct reader_traits
		{
			struct ctx_info
			{
				template<class ConnectionT>
				static self_t* get_context(ConnectionT *c)
				{
					self_t *self = static_cast<self_t*>(c->ev());
					return self;
				}

				template<class ConnectionT>
				static events_t* get_events(ConnectionT *c)
				{
					self_t *self = static_cast<self_t*>(c->ev());
					return self->events;
				}
			};

			struct mmc_read
			{
				template<class ConnectionT>
				static char const* fetch_message(ConnectionT *c, str_ref const& b)
				{
					return mmc_reader_fetch_message(b);
				}

				template<class ConnectionT>
				static size_t max_message_length(ConnectionT *c)
				{
					Derived *d = static_cast<Derived*>(ctx_info::get_context(c));
					return d->mmc_reader_max_message_length(c);
				}
			};
		};

		typedef mmc_reader_operations<reader_traits> reader_ops;

	private:

		virtual buffer_ref get_buffer(connection_t *c)
		{
			return reader_ops::get_buffer(c);
		}

		virtual rd_consume_status_t consume_buffer(connection_t *c, buffer_ref b, bool is_closed)
		{
			return reader_ops::read_process_buffer_data(c, b, is_closed);
		}

		virtual void on_closed(connection_t *c, io_close_report_t const& r)
		{
			events->on_closed(c, r);
		}

	public: // default implementations for the customization points

		template<class ConnectionT>
		size_t mmc_reader_max_message_length(ConnectionT *c)
		{
			return 1024;
		}

	public:

		events_t *events;

		mmc_reader_t(events_t *ev = NULL)
			: events(ev)
		{
		}
	};

	struct mmc_reader__dynamic_t : public mmc_reader_t<mmc_reader__dynamic_t>
	{
		size_t max_message_length;

		mmc_reader__dynamic_t(mmc_reader_events_t *ev = NULL, size_t const l = 1024)
			: mmc_reader_t<mmc_reader__dynamic_t>(ev)
			, max_message_length(l)
		{
		}

		template<class ConnectionT>
		size_t mmc_reader_max_message_length(ConnectionT *c)
		{
			return max_message_length;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__MMC_CONNECTION_IMPL_HPP_

