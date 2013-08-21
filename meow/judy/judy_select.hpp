////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2008 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_JUDY__JUDY_SELECT_HPP_
#define MEOW_JUDY__JUDY_SELECT_HPP_

#include <type_traits>

#include <meow/utility/nested_type_checker.hpp> 	// used to enable traits for judy_HS
#include <meow/judy/judy.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////
// selecting the appropriate ops from the key type

	template<class T, class Enabler = void>
	struct judy_ops_select
	{
		struct unsupported_judy_array_type_needs_traits_defined;
		typedef unsupported_judy_array_type_needs_traits_defined type;
	};

	template<class T>
	struct judy_ops_select<T, typename std::enable_if<std::is_integral<T>::value>::type>
	{
		typedef judy_ops_L type;
	};

	// SL is for null terminated strings
	//  so only enable that for [signed/unsigned] char* and any cv combo on it
	//  enabler needs some improvement
	template<class T>
	struct judy_ops_select<
		  T
		, typename std::enable_if<
			(   std::is_convertible<T, uint8_t const*>::value
				|| std::is_convertible<T, char const*>::value
				|| std::is_convertible<T, signed char const*>::value
				|| std::is_convertible<T, unsigned const*>::value
				)
			&& boost::is_pointer<T>::value
		  >::type
		>
	{
		typedef judy_ops_SL type;
	};

// ops HS uses a generic blob as key
//  so we can use it for everything, given the mechanism
//  to get data ptr and data length

	MEOW_DEFINE_NESTED_TYPE_CHECKER(judy_ops_check_nested_HS_key, key_t);

	template<class T>
	struct judy_ops_select<T, typename std::enable_if<judy_ops_check_nested_HS_key<judy_ops_HS_traits<T> >::value>::type>
	{
		typedef judy_ops_HS<T> type;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	struct judy_select
	{
		typedef typename judy_ops_select<T>::type 	operations_t;
		typedef typename operations_t::handle_t 	handle_t;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_JUDY__JUDY_SELECT_HPP_

