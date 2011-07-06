////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__TICKER_HPP_
#define MEOW_LIBEV__TICKER_HPP_

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <meow/unix/time.hpp>
#include <meow/libev/libev.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct ticker_t : private boost::noncopyable
	{
		typedef ticker_t self_t;
		typedef boost::function<void(self_t*, ev_tstamp)> callback_t;

		ticker_t(evloop_t *l)
			: loop_(l)
		{
			ev_init(&ev_, &libev_cb);
			ev_.data = this;
		}

		~ticker_t()
		{
			this->stop();
		}

		evloop_t* loop()
		{
			return loop_;
		}

		void start(os_timeval_t const& period, callback_t const& cb)
		{
			ev_timer_stop(loop_, &ev_);
			ev_init(&ev_, &libev_cb);

			ev_.repeat = os_timeval_to_double(period);
			ev_timer_again(loop_, &ev_);

			callback_ = cb;
		}

		void stop()
		{
			ev_timer_stop(loop_, &ev_);
			callback_ = callback_t();
		}

		void tick_once(os_timeval_t const& after, callback_t const& cb)
		{
			ev_timer_stop(loop_, &ev_);
			ev_init(&ev_, &libev_cb);
			ev_timer_set(&ev_, os_timeval_to_double(after), 0.);
			ev_timer_start(loop_, &ev_);

			callback_ = cb;
		}

	private:

		void cb(int revents)
		{
			if (callback_)
				callback_(this, ev_now(loop_));

			// ev_timer_again(loop_, &ev_);
		}

		static void libev_cb(evloop_t *loop, evtimer_t *ev, int revents)
		{
			static_cast<self_t*>(ev->data)->cb(revents);
		}

	private:
		evloop_t 	*loop_;
		evtimer_t 	ev_;
		callback_t 	callback_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__TICKER_HPP_

