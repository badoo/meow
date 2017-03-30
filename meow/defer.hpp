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

	struct defer_wrapper_t
	{
		mutable std::function<void()> cleaner;

		~defer_wrapper_t()
		{
			if (cleaner)
				cleaner();
		}
	};


	#define MEOW_DEFER_EX(body) \
		auto const& BOOST_PP_CAT(defer_wrapper___,__LINE__) = meow::defer_wrapper_t { .cleaner = body }; \
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
