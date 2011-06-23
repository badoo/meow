////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__IO_CONTEXT_HPP_
#define MEOW_LIBEV__IO_CONTEXT_HPP_

#include <unistd.h> 		// for ::close()

#include <meow/libev/libev.hpp>
#include <meow/utility/offsetof.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct io_context_t
	{
		static int const null_fd = -1;

		io_context_t() { this->set_fd(null_fd); }
		explicit io_context_t(int fd) { this->set_fd(fd); }

		~io_context_t() { this->reset_fd(); }
	
	public:

		bool is_valid() const { return null_fd != fd(); }

	public:

		int fd() const { return evt_.fd; }

		int set_fd(int new_fd)
		{
			int fd_ = evt_.fd;
			evt_.fd = new_fd;
			return fd_;
		}

		int release_fd()
		{
			return set_fd(null_fd);
		}

		void reset_fd(int new_fd = null_fd)
		{
			if (this->is_valid())
			{
				::close(fd());
			}
			set_fd(new_fd);
		}

		evio_t*       event()       { return &evt_; }
		evio_t const* event() const { return &evt_; }

	public:

		void* data() const
		{
			return event()->data;
		}

		void reset_data(void *new_data = NULL)
		{
			event()->data = new_data;
		}

		static io_context_t* cast_from_event(evio_t *ev)
		{
			return MEOW_SELF_FROM_MEMBER(io_context_t, evt_, ev);
		}

	private:
		evio_t evt_;
	};

	struct io_timed_context_t : public io_context_t
	{
		io_timed_context_t() {}
		io_timed_context_t(int fd) : io_context_t(fd) {}

		evtimer_t*       timer()       { return &timer_; }
		evtimer_t const* timer() const { return &timer_; }

		static io_timed_context_t* cast_from_event(evio_t *ev)
		{
			return static_cast<io_timed_context_t*>(io_context_t::cast_from_event(ev));
		}

		static io_timed_context_t* cast_from_timer(evtimer_t *timer)
		{
			return MEOW_SELF_FROM_MEMBER(io_timed_context_t, timer_, timer);
		}

	private:
		evtimer_t timer_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__IO_CONTEXT_HPP_

