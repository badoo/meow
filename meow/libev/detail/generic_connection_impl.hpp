////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_IMPL_HPP_
#define MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_IMPL_HPP_

#include <meow/unix/fcntl.hpp>
#include <meow/utility/nested_name_alias.hpp>

#include <meow/libev/libev.hpp>
#include <meow/libev/io_machine.hpp>
#include <meow/libev/detail/generic_connection.hpp>
#include <meow/libev/detail/generic_connection_traits.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class Interface
		, class Traits
		, class Events = typename Interface::events_t
		>
	struct generic_connection_impl_t
		: public Interface
		, public Traits::read::context_t
	{
		typedef generic_connection_impl_t 		self_t;
		typedef generic_connection_impl_t 		base_t;
		typedef Events							events_t;

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

		evloop_t 		*loop_;
		io_context_t 	io_;
		events_t 		*ev_;

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

//
// defines a connection wrapper like mmc_connection_t
// example: 
//  MEOW_LIBEV_DEFINE_CONNECTION_WRAPPER(mmc_connection_impl_t, mmc_connection_repack_traits, mmc_connection_t);
//
#define MEOW_LIBEV_DEFINE_CONNECTION_WRAPPER(name, traits, base_interface) 	\
	template<class Traits, class InterfaceT = base_interface >				\
	struct name 															\
		: public generic_connection_impl_t<									\
					  InterfaceT											\
					, traits<Traits>										\
					, typename base_interface::events_t						\
					>														\
	{																		\
		name(evloop_t *loop, int fd, typename base_interface::events_t *ev)	\
			: name::base_t(loop, fd, ev)									\
		{																	\
		}																	\
	};																		\
/**/

// used from inside connection traits to call callback through connection->events
#define MEOW_LIBEV_GENERIC_CONNECTION_CTX_CALLBACK(ctx, cb_name, args...) \
	ctx->ev()->cb_name(ctx, args)

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_DETAIL__GENERIC_CONNECTION_IMPL_HPP_

