////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__SIGNAL_EVENTS_IMPL_HPP_
#define MEOW_LIBEV__SIGNAL_EVENTS_IMPL_HPP_

#include <cassert>
#include <csignal>

#include <meow/libev/libev.hpp>
#include <meow/libev/signal_events.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct signal_events_impl_t : public signal_events_t
	{
		typedef signal_events_impl_t self_t;

		struct item_t
		{
			evsignal_t ev;
			callback_t cb;
		};

		item_t i_[max_signals];
		evloop_t *loop_;

	private:

		static void empty_handler(signal_events_t*, int) {}

		item_t* item_get(int signo)
		{
			assert((0 <= signo) && (signo < max_signals));
			return &i_[signo];
		}

		static void libev_cb(evloop_t *loop, evsignal_t *w, int revents)
		{
			self_t *self = static_cast<self_t*>(w->data);
			self->cb(w);
		}

		void cb(evsignal_t *w)
		{
			item_t *item = item_get(w->signum);
			assert(&item->ev == w);

			item->cb(this, w->signum);
		}

	public:

		virtual evloop_t* loop() const
		{
			return loop_;
		}

		virtual void handle(int signo, callback_t const& cb) override
		{
			item_t *item = item_get(signo);
			evsignal_t *ev = &item->ev;

			assert(signo == ev->signum);
			item->cb = cb;

			if (!ev_is_active(ev))
				ev_signal_start(loop_, ev);
		}

		virtual void ignore(int signo)
		{
			this->handle(signo, &self_t::empty_handler);
		}

		virtual void as_default(int signo)
		{
			item_t *item = item_get(signo);
			evsignal_t *ev = &item->ev;

			if (ev_is_active(ev))
				ev_signal_stop(loop_, ev);

			std::signal(signo, SIG_DFL);
		}

		virtual void reset()
		{
			for (int i = 0; i < max_signals; ++i)
			{
				this->as_default(i);
			}
		}

	public:

		signal_events_impl_t(evloop_t *l)
			: loop_(l)
		{
			for (int i = 0; i < max_signals; ++i)
			{
				item_t *item = item_get(i);
				ev_signal_init(&item->ev, &self_t::libev_cb, i);
				item->ev.data = this;
			}
		}

		virtual ~signal_events_impl_t()
		{
			// this object is supposed to exist for the whole lifetime of the program
			//  so this reset call is just polluting my strace output
			// this->reset();
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class L>
	signal_events_move_ptr create_signal_events(L& loop)
	{
		return signal_events_move_ptr(new signal_events_impl_t(get_handle(loop)));
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__SIGNAL_EVENTS_IMPL_HPP_

