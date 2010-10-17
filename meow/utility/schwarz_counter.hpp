////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UTILITY__SCHWARZ_COUNTER_HPP_
#define MEOW_UTILITY__SCHWARZ_COUNTER_HPP_

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits>
	struct schwarz_counter_t
	{
		schwarz_counter_t()
		{
			if (0 == counter_ref()++)
				Traits::init();
		}

		~schwarz_counter_t()
		{
			if (0 == --counter_ref())
				Traits::fini();
		}

	private:
		static size_t& counter_ref(size_t const init_cnt = 0)
		{
			static size_t cnt_ = init_cnt;
			return cnt_;
		}
	};

#define MEOW_SCHWARZ_COUNTER_INITIALIZER(name)	\
	struct name##_init_traits				\
	{										\
		static void init();					\
		static void fini();					\
	};										\
	/* const is vital here, giving this variable internal linkage 	*/	\
	/* thus every translation unit will have it's own copy			*/	\
	meow::schwarz_counter_t<name##_init_traits> const name##_schwarz_initializer_;	\
/**/

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UTILITY__SCHWARZ_COUNTER_HPP_

