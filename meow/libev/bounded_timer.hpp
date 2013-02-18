////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__BOUNDED_TIMER_HPP_
#define MEOW_LIBEV__BOUNDED_TIMER_HPP_

#include <functional> // std::function

#include <boost/noncopyable.hpp>
#include <boost/intrusive/list.hpp>

#include <meow/std_unique_ptr.hpp>
#include <meow/libev/libev_fwd.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct bounded_timer_t : private boost::noncopyable
	{
		typedef bounded_timer_t  self_t;
		typedef double           timestamp_t;

		struct node_t;
		typedef std::function<void(self_t*, node_t*, timestamp_t)> callback_t;

		typedef boost::intrusive::list_base_hook<
					  boost::intrusive::link_mode<boost::intrusive::auto_unlink>
					> node_hook_t;

		struct node_t : public node_hook_t
		{
			callback_t timer_callback;
		};

	public:

		virtual ~bounded_timer_t() {}

		virtual evloop_t* loop() const = 0;

		virtual size_t tick_interval_msec() const = 0;
		virtual size_t max_timeout_intervals() const = 0;

		virtual void timer_set(node_t*, timestamp_t) = 0;
		virtual void timer_del(node_t*) = 0;
	};

	typedef bounded_timer_t::node_t       bounded_timer_node_t;
	typedef bounded_timer_t::callback_t   bounded_timer_callback_t;
	typedef bounded_timer_t::timestamp_t  bounded_timer_timestamp_t;

	typedef std::unique_ptr<bounded_timer_t>       bounded_timer_ptr;
	typedef std::unique_ptr<bounded_timer_node_t>  bounded_timer_node_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__BOUNDED_TIMER_HPP_

