////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV__CONNECTION_SEND_FMT_HPP_
#define MEOW_LIBEV__CONNECTION_SEND_FMT_HPP_

#include <meow/format/format_functions.hpp>
#include <meow/format/sink/buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	#define DEFINE_CONNECTION_SEND_FMT_BODY(z, n, fn_name) 		\
	template<class C, class F FMT_TEMPLATE_PARAMS(n)> 		\
	inline void connection_send_fmt_l( 						\
			  C *c 											\
			, size_t const init_sz 							\
			, F const& fmt 									\
			  FMT_DEF_PARAMS(n)) 							\
	{ 														\
		buffer_move_ptr b(new buffer_t(init_sz)); 			\
		format::fmt(*b, fmt FMT_CALL_SITE_ARGS(n)); 		\
		c->send(move(b)); 									\
	} 														\
	template<class C, class F FMT_TEMPLATE_PARAMS(n)> 		\
	inline void connection_send_fmt(C *c, F const& fmt FMT_DEF_PARAMS(n)) 	\
	{ 														\
		connection_send_fmt_l(c, 1024, fmt FMT_CALL_SITE_ARGS(n)); 	\
	} 														\
	/**/

	BOOST_PP_REPEAT(32, DEFINE_CONNECTION_SEND_FMT_BODY, _)

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV__CONNECTION_SEND_FMT_HPP_

