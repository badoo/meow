////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__FORMAT_TO_BUFFER_HPP_
#define MEOW_FORMAT__FORMAT_TO_BUFFER_HPP_

#include "sink/buffer.hpp"
#include "format_functions.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_FORMAT_DEFINE_FMT_BUF(z, n, d) 					\
template<class F FMT_TEMPLATE_PARAMS(n)> 						\
inline buffer_move_ptr fmt_buf( 								\
		  size_t initial_sz 									\
		, F const& fmt_str 										\
		  FMT_DEF_PARAMS(n)) 									\
{ 																\
	buffer_move_ptr buf = create_buffer(initial_sz);			\
	fmt(*buf, fmt_str FMT_CALL_SITE_ARGS(n)); 					\
	return move(buf); 											\
} 																\
/**/

BOOST_PP_REPEAT_FROM_TO(0, 32, MEOW_FORMAT_DEFINE_FMT_BUF, _);


#define MEOW_FORMAT_DEFINE_WRITE_BUF(z, n, d) 					\
template<class F FMT_TEMPLATE_PARAMS(n)> 						\
inline buffer_move_ptr write_buf( 								\
		  size_t initial_sz 									\
		  FMT_DEF_PARAMS(n)) 									\
{ 																\
	buffer_move_ptr buf = create_buffer(initial_sz);			\
	write(*buf FMT_CALL_SITE_ARGS(n)); 							\
	return move(buf); 											\
} 																\
/**/

BOOST_PP_REPEAT_FROM_TO(1, 32, MEOW_FORMAT_DEFINE_WRITE_BUF, _);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_TO_BUFFER_HPP_

