////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__FORMAT_TMP_HPP_
#define MEOW_FORMAT__FORMAT_TMP_HPP_

#include <meow/tmp_buffer.hpp>

#include "sink/char_buffer.hpp"
#include "format_functions.hpp"

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
	return sink.used_part(); 									\
} 																\
/**/

BOOST_PP_REPEAT_FROM_TO(0, 32, MEOW_FORMAT_DEFINE_FMT_TMP, _);

#define MEOW_FORMAT_DEFINE_WRITE_TMP(z, n, d) 					\
template<size_t N FMT_TEMPLATE_PARAMS(n)> 						\
inline str_ref write_tmp( 										\
		  FMT_DEF_PARAMS_W(n) 									\
		, tmp_buffer<N> const& buf = tmp_buffer<N>()) 			\
{ 																\
	sink::char_buffer_sink_t sink(buf.get(), buf.size()); 		\
	write(sink FMT_CALL_SITE_ARGS(n)); 							\
	return sink.used_part(); 									\
} 																\
/**/

BOOST_PP_REPEAT_FROM_TO(1, 32, MEOW_FORMAT_DEFINE_WRITE_TMP, _);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_TMP_HPP_

