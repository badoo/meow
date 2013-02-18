////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__SIGNAL_EVENTS_HPP_
#define MEOW_LIBEV__SIGNAL_EVENTS_HPP_

#include <functional> // function
#include <boost/noncopyable.hpp>

#include <meow/std_unique_ptr.hpp>
#include <meow/libev/libev_fwd.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_INTERNAL_LIBEV_NSIG
#	define MEOW_INTERNAL_LIBEV_NSIG 64
#endif

	struct signal_events_t : private boost::noncopyable
	{
		enum { max_signals = MEOW_INTERNAL_LIBEV_NSIG };

		typedef signal_events_t 					self_t;
		typedef std::function<void(self_t*, int)>	callback_t;

	public:

		virtual evloop_t* loop() const = 0;

		virtual void handle(int signo, callback_t const&) = 0; // overwrites old handler
		virtual void ignore(int signo) = 0;
		virtual void as_default(int signo) = 0;

		virtual void reset() = 0;

		virtual ~signal_events_t() {}
	};

	typedef std::unique_ptr<signal_events_t> signal_events_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__SIGNAL_EVENTS_HPP_

