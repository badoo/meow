////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__BOUNDED_TIMER_IMPL_HPP_
#define MEOW_LIBEV__BOUNDED_TIMER_IMPL_HPP_

#include <array>
#include <functional> // bind

#include <meow/libev/bounded_timer.hpp>
#include <meow/libev/libev.hpp>
#include <meow/libev/ticker.hpp>
#include <meow/unix/time.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  size_t max_timeout_in_ticks       // max item timeout, in tick intervals
		, size_t tick_interval_ms = 1000	// length of tick interval, in milliseconds
	>
	struct bounded_timer_impl_t : public bounded_timer_t
	{
		typedef bounded_timer_t       base_t;
		typedef bounded_timer_impl_t  self_t;

		typedef typename base_t::node_t       node_t;
		typedef typename base_t::timestamp_t  timestamp_t;

		typedef boost::intrusive::list<
					  node_t
					, boost::intrusive::base_hook<node_hook_t>
					, boost::intrusive::constant_time_size<false>
					> timer_list_t;

		typedef std::array<timer_list_t, max_timeout_in_ticks> holder_t;

		enum {
			  max_timeout = max_timeout_in_ticks
			, tick_interval = tick_interval_ms
		};

	public:

		bounded_timer_impl_t(evloop_t *loop)
			: index_(0)
			, ticker_(loop)
		{
			// antoxa: best syntax i could come up with for gcc 4.7.2
			auto const tv = timeval_t {
				.tv_sec = tick_interval_ms / msec_in_sec,
				.tv_nsec = (tick_interval_ms % msec_in_sec) * (nsec_in_sec / msec_in_sec),
			};

			ticker_.start(tv, std::bind(&self_t::on_tick, this, std::placeholders::_1, std::placeholders::_2));
		}

		~bounded_timer_impl_t()
		{
			this->shutdown();
		}

		void shutdown()
		{
			ticker_.stop();
		}

	public:

		virtual evloop_t* loop() const { return ticker_.loop(); }

		virtual size_t tick_interval_msec() const { return tick_interval_ms; }
		virtual size_t max_timeout_intervals() const { return max_timeout_in_ticks; }

		virtual void timer_set(node_t *node, timestamp_t ts)
		{
			size_t offset = make_offset_from_timestamp(ts);

			// zero is a special value, meaning 'next tick'
			//  this is essentialy 1
			if (0 == offset)
				offset = 1;

			// the destination list, we're going to add item into (at the end)
			timer_list_t& dst_l = l_[to_index(index_ + offset)];

			// push, if it was already linked -> like we care
			if (node->is_linked())
				node->unlink();

			dst_l.push_back(*node);
		}

		virtual void timer_del(node_t *node)
		{
			if (node->is_linked())
				node->unlink();
		}

	private:

		static inline size_t make_offset_from_timestamp(timestamp_t ts)
		{
			timeval_t const tv = timeval_from_double(ts);
			uint64_t const msec = tv.tv_sec * msec_in_sec + tv.tv_nsec / (nsec_in_sec / msec_in_sec);

			size_t const offset = msec / tick_interval_ms;
			assert(offset < max_timeout_in_ticks);

			return offset;
		}

		static inline size_t to_index(size_t sz)
		{
			return sz % max_timeout_in_ticks;
		}

		void on_tick(ticker_t*, double now)
		{
			index_ = to_index(index_ + 1);
			timer_list_t& l = l_[index_];

			while (!l.empty())
			{
				// no point in implementation that doesn't delete list elements anyway
				//  because ideally the client doesn't care about the size of our internal
				//  array-of-lists holder
				node_t *node = &l.front();
				l.pop_front();

				node->timer_callback(this, node, now);
			}
		}

	private:
		size_t    index_;
		ticker_t  ticker_;
		holder_t  l_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__BOUNDED_TIMER_IMPL_HPP_

