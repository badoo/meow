////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__DEFER_HPP_
#define MEOW__DEFER_HPP_

#include <functional>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class F>
	struct defer_wrapper_t
	{
		F *cleaner;

		~defer_wrapper_t()
		{
			(*cleaner)();
		}
	};

	template<class F>
	inline defer_wrapper_t<F> make_defer_wrapper(F *func)
	{
		return { .cleaner = func };
	}


	#define MEOW_DEFER_EX(body) \
		auto const BOOST_PP_CAT(defer_func___,__LINE__) = body ; \
		auto const BOOST_PP_CAT(defer_wrapper___,__LINE__) = meow::make_defer_wrapper(& BOOST_PP_CAT(defer_func___,__LINE__) );
	/**/

	#define MEOW_DEFER(body) \
		MEOW_DEFER_EX([&]() -> void { body; })
	/**/

////////////////////////////////////////////////////////////////////////////////////////////////

	struct meow_defer_ctx_t
	{
		std::vector<std::function<void()>> cleanups;

		template<class Function>
		void add(Function const& func)
		{
			cleanups.emplace_back(func);
		}

		~meow_defer_ctx_t()
		{
			if (cleanups.empty())
				return;

			for (auto i = cleanups.size() - 1; i != 0; --i)
				cleanups[i]();
		}
	};

	#define MEOW_DEFER_ADD_EX(ctx, body)  \
		(ctx).add(body);                    \
	/**/

	#define MEOW_DEFER_ADD(ctx, body)                    \
		MEOW_DEFER_ADD_EX(ctx, [&]() -> void { body; })  \
	/**/

////////////////////////////////////////////////////////////////////////////////////////////////
} //namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__DEFER_HPP_
