////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__FORMAT_TMP_HPP_
#define MEOW_FORMAT__FORMAT_TMP_HPP_

#include "sink/char_buffer.hpp"
#include "sink/std_string.hpp"
#include "format_functions.hpp"

#include <meow/tmp_buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_FORMAT_DEFINE_FMT_TMP(z, n, d) 					\
template<size_t N, class F FMT_TEMPLATE_PARAMS(n)> 				\
inline str_ref fmt_tmp( 										\
		F const& fmt_str 										\
		FMT_DEF_PARAMS(n) 										\
		, tmp_buffer<N> const& buf = tmp_buffer<N>()) 			\
{ 																\
	sink::char_buffer_sink_t sink(buf.get(), buf.size()); 		\
	fmt(sink, fmt_str FMT_CALL_SITE_ARGS(n)); 					\
	return str_ref(buf.get(), sink.size()); 					\
} 																\
/**/

BOOST_PP_REPEAT_FROM_TO(0, 32, MEOW_FORMAT_DEFINE_FMT_TMP, _);

////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_FORMAT_DEFINE_FMT_STR(z, n, d) 					\
template<class F FMT_TEMPLATE_PARAMS(n)> 						\
inline std::string fmt_str( 									\
		F const& fmt_str 										\
		FMT_DEF_PARAMS(n) 										\
		) 														\
{ 																\
	std::string result; 										\
	return fmt(result, fmt_str FMT_CALL_SITE_ARGS(n)); 			\
} 																\
/**/

BOOST_PP_REPEAT_FROM_TO(0, 32, MEOW_FORMAT_DEFINE_FMT_STR, _);

////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_DEFINE_WRITE_STR_FN(z, n, d)								\
	template<FMT_TEMPLATE_PARAMS_W(n)>									\
	inline std::string write_str(FMT_DEF_PARAMS_W(n)) 					\
	{																	\
		std::string result;												\
		return write(result FMT_CALL_SITE_ARGS(n));						\
	}																	\
/**/

	BOOST_PP_REPEAT_FROM_TO(1, 32, MEOW_DEFINE_WRITE_STR_FN, _);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_TMP_HPP_

