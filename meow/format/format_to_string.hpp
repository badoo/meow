////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__FORMAT_TO_STRING_HPP_
#define MEOW_FORMAT__FORMAT_TO_STRING_HPP_

#include "format.hpp"
#include "sink/std_string.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class F, class... A>
	inline std::string fmt_str(F const& f, A const&... args)
	{
		std::string result;
		return fmt(result, f, args...);
	}

	template<class... A>
	inline std::string write_str(A const&... args)
	{
		std::string result;
		return write(result, args...);
	}

#if 0
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
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_TO_STRING_HPP_

