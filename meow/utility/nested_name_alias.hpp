////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UTILITY__NESTED_NAME_ALIAS_HPP_
#define MEOW_UTILITY__NESTED_NAME_ALIAS_HPP_

#include <type_traits> // std::conditional, std::is_same

#include <boost/preprocessor/cat.hpp>

#include <meow/utility/nested_type_checker.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

// MEOW_DEFINE_NESTED_NAME_READER(reader_name, nested_anem):
//  this macro gives information about Tr::nested_type
//  exists: nested_name is defined in Tr
//  is_void: exists && is equal to void
//  type: the type itself or void if it doesn't exist
//

#define MEOW_DEFINE_NESTED_NAME_READER(reader_name, nested_name) 			\
	template<class NESTED_READER_Tr>										\
	struct reader_name {													\
		MEOW_DEFINE_NESTED_TYPE_CHECKER(check_nested_name, nested_name);	\
		template<bool b, class Inner_Tr> struct inner_if;					\
		template<class Inner_Tr> struct inner_if<true, Inner_Tr> {			\
			typedef typename Inner_Tr::nested_name type;					\
		};																	\
		template<class Inner_Tr> struct inner_if<false, Inner_Tr> {			\
			typedef void type;												\
		};																	\
		enum { exists = check_nested_name<NESTED_READER_Tr>::value };		\
		typedef inner_if<exists, NESTED_READER_Tr> inner_t;					\
		enum { is_void = exists && std::is_same<void, typename inner_t::type>::value };		\
		typedef typename inner_t::type type;								\
	};																		\
/**/

////////////////////////////////////////////////////////////////////////////////////////////////

// 
// MEOW_DEFINE_NESTED_NAME_ALIAS_EX(traits, nested_name, new_name, none_type, void_type):
// this macro checks if there is a nested traits::nested_name
//  and typedefs the proper local 'new_name' as follows
//  * traits::nested_name doesn't exist 		-> none_type
//  * traits::nested_name exists and is void 	-> void_type
//  * else 										-> typename traits::nested_name
//
// NOTE: using inner_if magic here, because we can't refer to traits::nested_name
//  until it's known to exist, and that's checked in outer_if
//
//
// MEOW_DEFINE_NESTED_NAME_ALIAS(traits, nested_name, none_type, void_type):
//  same as above, but new_name = nested_name

#define MEOW_DEFINE_NESTED_NAME_ALIAS_II(checker_name, nested_name, none_type, void_type) 		\
	template<class NAME_ALIAS_Tr>											\
	struct checker_name {													\
		MEOW_DEFINE_NESTED_NAME_READER(name_reader_t, nested_name);			\
		typedef name_reader_t<NAME_ALIAS_Tr> name_reader;					\
		typedef 															\
			typename std::conditional<										\
					  !name_reader::exists									\
					, none_type 											\
					, typename std::conditional< 							\
						  name_reader::is_void 								\
						, void_type 										\
						, typename name_reader::type						\
						>::type												\
					>::type type; 											\
	};																		\
/**/

#define MEOW_DEFINE_NESTED_NAME_ALIAS_I(traits, checker_name, nested_name, new_name, none_type, void_type) 	\
	MEOW_DEFINE_NESTED_NAME_ALIAS_II(checker_name, nested_name, none_type, void_type) 		\
	typedef typename checker_name<traits>::type new_name;								\
/**/

#define MEOW_DEFINE_NESTED_NAME_ALIAS_EX(traits, nested_name, new_name, none_type, void_type) 	\
	MEOW_DEFINE_NESTED_NAME_ALIAS_I(  traits 											\
									, BOOST_PP_CAT(nested_name_redefiner_, __LINE__) 	\
									, nested_name, new_name, none_type, void_type 		\
									) 													\
/**/

#define MEOW_DEFINE_NESTED_NAME_ALIAS(traits, nested_name, none_type, void_type) 		\
	MEOW_DEFINE_NESTED_NAME_ALIAS_I(  traits 											\
									, BOOST_PP_CAT(nested_name_redefiner_, __LINE__) 	\
									, nested_name, nested_name, none_type, void_type 	\
									) 													\
/**/

// useful when you need to make a default, but also to give the user an option of disabling the thing completely
#define MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE_EX(traits, nested_name, new_name, none_type) \
	MEOW_DEFINE_NESTED_NAME_ALIAS_EX(traits, nested_name, new_name, none_type, void)

#define MEOW_DEFINE_NESTED_NAME_ALIAS_OR_MY_TYPE(traits, nested_name, none_type) \
	MEOW_DEFINE_NESTED_NAME_ALIAS(traits, nested_name, none_type, void)

#define MEOW_DEFINE_NESTED_NAME_ALIAS_OR_VOID(traits, nested_name) \
	MEOW_DEFINE_NESTED_NAME_ALIAS(traits, nested_name, void, void)

#endif // MEOW_UTILITY__NESTED_NAME_ALIAS_HPP_

