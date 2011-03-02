////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__FORMAT_TO_BUFFER_HPP_
#define MEOW_FORMAT__FORMAT_TO_BUFFER_HPP_

#include "sink/char_buffer.hpp"
#include "format_functions.hpp"

#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_DEFINE_FMT_TO_BUFFER_FN(z, n, d)				\
	template<class F FMT_TEMPLATE_PARAMS(n)>			\
	inline buffer_ref fmt_to_buffer(buffer_ref *to, F const& fmt_str FMT_DEF_PARAMS(n)) 	\
	{																				\
		sink::char_buffer_sink_t sink(to->begin(), to->size());						\
		fmt(sink, fmt_str FMT_CALL_SITE_ARGS(n));									\
		return buffer_ref(to->begin(), sink.size());								\
	}																				\
/**/

	BOOST_PP_REPEAT_FROM_TO(0, 32, MEOW_DEFINE_FMT_TO_BUFFER_FN, _);

#define MEOW_DEFINE_WRITE_TO_BUFFER_FN(z, n, d)							\
	template<FMT_TEMPLATE_PARAMS_W(n)>									\
	inline buffer_ref write_to_buffer(buffer_ref *to FMT_DEF_PARAMS(n)) \
	{																	\
		sink::char_buffer_sink_t sink(to->begin(), to->size());			\
		write(sink FMT_CALL_SITE_ARGS(n));								\
		return buffer_ref(to->begin(), sink.size());					\
	}																	\
/**/

	BOOST_PP_REPEAT_FROM_TO(1, 32, MEOW_DEFINE_WRITE_TO_BUFFER_FN, _);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_TO_BUFFER_HPP_

