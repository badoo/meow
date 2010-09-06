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

#define MEOW_FORMAT_DEFINE_FMT_TMP_SKEL(n, decl_spec, fn_name) \
template<size_t N, class F FMT_TEMPLATE_PARAMS(n)> 				\
decl_spec str_ref fn_name( 										\
		F const& fmt_str 										\
		FMT_DEF_PARAMS(n) 										\
		, tmp_buffer<N> const& buf = tmp_buffer<N>()) 			\
{ 																\
	sink::char_buffer_sink_t sink(buf.get(), buf.size()); 		\
	fmt(sink, fmt_str FMT_CALL_SITE_ARGS(n)); 					\
	return str_ref(buf.get(), sink.size()); 					\
} 																\
/**/

#define MEOW_FORMAT_DEFINE_FMT_TMP(z, n, d) 					\
	MEOW_FORMAT_DEFINE_FMT_TMP_SKEL(n, inline, fmt_tmp)

BOOST_PP_REPEAT_FROM_TO(0, 32, MEOW_FORMAT_DEFINE_FMT_TMP, _);

////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_FORMAT_DEFINE_FMT_STR_SKEL(n, decl_spec, fn_name) \
template<class F FMT_TEMPLATE_PARAMS(n)> 						\
decl_spec std::string fn_name( 									\
		F const& fmt_str 										\
		FMT_DEF_PARAMS(n) 										\
		) 														\
{ 																\
	std::string result; 										\
	fmt(result, fmt_str FMT_CALL_SITE_ARGS(n)); 				\
	return result; 												\
} 																\
/**/

#define MEOW_FORMAT_DEFINE_FMT_STR(z, n, d) 					\
	MEOW_FORMAT_DEFINE_FMT_STR_SKEL(n, inline, fmt_str) 		\
/**/

BOOST_PP_REPEAT_FROM_TO(0, 32, MEOW_FORMAT_DEFINE_FMT_STR, _);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_TMP_HPP_

