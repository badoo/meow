////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__FORMAT_TO_STR_COPY_HPP_
#define MEOW_FORMAT__FORMAT_TO_STR_COPY_HPP_

#include <meow/str_copy.hpp>
#include "format_functions.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	namespace sink {

		template<class CharT, class Traits>
		struct sink_write<string_copy<CharT, Traits> >
		{
			static void call(
					  string_copy<CharT, Traits>& sink
					, size_t total_len
					, string_ref<CharT const> const *slices
					, size_t n_slices
					)
			{
				sink.reset_and_resize_to(total_len);

				char *to = sink.data();
				for (size_t i = 0; i < n_slices; ++i)
				{
					memcpy(to, slices[i].data(), slices[i].size());
					to += slices[i].size();
				}
			}
		};
	}

////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_FORMAT_DEFINE_FMT_STR_COPY(z, n, d) 				\
template<class F FMT_TEMPLATE_PARAMS(n)> 						\
inline str_copy fmt_str_copy( 									\
		F const& fmt_str 										\
		FMT_DEF_PARAMS(n) 										\
		) 														\
{ 																\
	str_copy result; 											\
	fmt(result, fmt_str FMT_CALL_SITE_ARGS(n)); 				\
	return result;												\
} 																\
/**/

BOOST_PP_REPEAT_FROM_TO(0, 32, MEOW_FORMAT_DEFINE_FMT_STR_COPY, _);

////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_FORMAT_DEFINE_WRITE_STR_COPY(z, n, d)						\
	template<FMT_TEMPLATE_PARAMS_W(n)>									\
	inline str_copy write_str(FMT_DEF_PARAMS_W(n)) 						\
	{																	\
		str_copy result;												\
		write(result FMT_CALL_SITE_ARGS(n));							\
		return result;													\
	}																	\
/**/

	BOOST_PP_REPEAT_FROM_TO(1, 32, MEOW_FORMAT_DEFINE_WRITE_STR_COPY, _);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_TO_STR_COPY_HPP_

