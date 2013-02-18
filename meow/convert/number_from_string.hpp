////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_CONVERT__NUMBER_FROM_STRING_HPP_
#define MEOW_CONVERT__NUMBER_FROM_STRING_HPP_

#include <typeinfo>  // std::bad_cast
#include <utility>   // enable_if
#include <type_traits>

#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct bad_number_from_string : public std::bad_cast
	{
		virtual char const* what() const throw()
		{
			return "number_from_string: can't cast source to target arithmetic type";
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T, class Enabler = void>
	struct number_from_string_caster_t;

	template<class T>
	struct number_from_string_caster_t<
			  T
			, typename std::enable_if<
				   std::is_integral<T>::value
				&& std::is_signed<T>::value
				&& (sizeof(T) <= sizeof(long))
			>::type
			>
	{
		static T cast(char const *b, char **e) { return ::strtol(b, e, 0); }
	};

	template<class T>
	struct number_from_string_caster_t<
			  T
			, typename std::enable_if<
				   std::is_integral<T>::value
				&& std::is_unsigned<T>::value
				&& (sizeof(T) <= sizeof(long))
			>::type
			>
	{
		static T cast(char const *b, char **e) { return ::strtoul(b, e, 0); }
	};

#define DEFINE_SIMPLE_NUMBER_CASTER(type, function) 	\
	template<> 											\
	struct number_from_string_caster_t<type> { 			\
		static type cast(char const *b, char **e) { return function(b, e, 0); } 	\
	}; 													\
/**/

#define DEFINE_SIMPLE_FLOATING_CASTER(type, function) 	\
	template<> 											\
	struct number_from_string_caster_t<type> { 			\
		static type cast(char const *b, char **e) { return function(b, e); } 	\
	}; 													\
/**/

	DEFINE_SIMPLE_NUMBER_CASTER(long long, 			::strtoll);
	DEFINE_SIMPLE_NUMBER_CASTER(unsigned long long, ::strtoull);
	DEFINE_SIMPLE_FLOATING_CASTER(float, 			::strtod);
	DEFINE_SIMPLE_FLOATING_CASTER(double, 			::strtod);
	DEFINE_SIMPLE_FLOATING_CASTER(long double, 		::strtold);

#undef DEFINE_SIMPLE_FLOATING_CASTER
#undef DEFINE_SIMPLE_NUMBER_CASTER

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class To>
	To number_from_string(char const *b, char const *e)
	{
		char *p = NULL;
		To const to = detail::number_from_string_caster_t<To>::cast(b, &p);
		if (p != e)
			throw bad_number_from_string();
		return to;
	}

	template<class To>
	To number_from_string(str_ref const& s)
	{
		return number_from_string<To>(s.begin(), s.end());
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_CONVERT__NUMBER_FROM_STRING_HPP_

