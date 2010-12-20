////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UTILITY__NESTED_NAME_ALIAS_HPP_
#define MEOW_UTILITY__NESTED_NAME_ALIAS_HPP_

#include <boost/type_traits/is_same.hpp>
#include <boost/preprocessor/cat.hpp>

#include <meow/utility/nested_type_checker.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

// this macro checks if there is a nested Traits::nested_name
//  and typedefs the proper local 'nested_name' as follows
//  * Traits::nested_name doesn't exist 		-> void
//  * Traits::nested_name exists and is void 	-> void
//  * else 										-> typename Traits::nested_name
//
// NOTE: using inner_if magic here, because we can't refer to Traits::nested_name
//  until it's known to exist, and that's checked in outer_if
//  no boost.mpl, because it increases compile time 3x :(
//

#define MEOW_DEFINE_NESTED_NAME_ALIAS_OR_VOID_I(checker_name, nested_name) 		\
	template<class Tr>														\
	struct checker_name {													\
		MEOW_DEFINE_NESTED_TYPE_CHECKER(check_nested_name, nested_name);	\
		template<bool b, class Inner_Tr> struct inner_if;					\
		template<class Inner_Tr> struct inner_if<true, Inner_Tr> {			\
			typedef typename Inner_Tr::nested_name type;					\
			enum { is_enabled = !boost::is_same<void, type>::value };		\
		};																	\
		template<class Inner_Tr> struct inner_if<false, Inner_Tr> {			\
			typedef void type;												\
			enum { is_enabled = false };									\
		};																	\
		typedef inner_if<check_nested_name<Tr>::value, Tr> inner_t;			\
		template<bool b, class NextT> struct outer_if;						\
		template<class NextT> struct outer_if<true, NextT> { typedef typename NextT::type type; };	\
		template<class NextT> struct outer_if<false, NextT> { typedef void type; };					\
		typedef typename outer_if<inner_t::is_enabled, inner_t>::type type;	\
	};																		\
	typedef typename checker_name<Traits>::type nested_name;				\
/**/

#define MEOW_DEFINE_NESTED_NAME_ALIAS_OR_VOID(nested_name) \
	MEOW_DEFINE_NESTED_NAME_ALIAS_OR_VOID_I(BOOST_PP_CAT(nested_name_redefiner_, __LINE__), nested_name)

#endif // MEOW_UTILITY__NESTED_NAME_ALIAS_HPP_

