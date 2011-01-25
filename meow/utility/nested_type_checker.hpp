////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UTILITY__NESTED_TYPE_CHECKER_HPP_
#define MEOW_UTILITY__NESTED_TYPE_CHECKER_HPP_

/*
Description: 
	helper macros to define traits checking for specific nested types/functions/values

Usage Example:
	MEOW_DEFINE_NESTED_TYPE_CHECKER(check_has_nested_type, type);
	struct test_okay_t { typedef int type; };
	struct test_okay_2_t { struct type {}; };
	struct test_fail_t {};
	int main()
	{
		printf("%d ", check_has_nested_type<test_okay_t>::value);	// prints: 1
		printf("%d ", check_has_nested_type<test_okay_2_t>::value); // prints: 1
		printf("%d ", check_has_nested_type<test_fail_t>::value);	// prints: 0
	}

To Detect nested template write:
	MEOW_DEFINE_NESTED_TYPE_CHECKER(check_has_nested_template, template nested_template_type<template_parameter>)

Usage Example 2:
	MEOW_DEFINE_NESTED_TYPE_CHECKER(check_has_nested_template, template test<int>)
	struct test_okay_t { template<class T> struct test {}; };
	int main()
	{
		printf("%d", check_has_nested_template<test_okay_t>::value); // prints: 1
	}
*/

#define MEOW_DEFINE_NESTED_MEMBER_CHECKER(checker_name, member) 			\
template<class T>															\
struct checker_name															\
{																			\
	struct no_t {};															\
	template<size_t n> struct wrap_t { no_t dummy[n]; };					\
	template<class U> static wrap_t<sizeof(&U::member)> test(void*);		\
	template<class U> static no_t test(...);								\
	enum { value = (sizeof(no_t) != sizeof(test<T>(NULL))) };				\
};																			\
/* ENDMACRO: MEOW_DEFINE_NESTED_MEMBER_CHECKER */

#define MEOW_DEFINE_NESTED_TYPE_CHECKER(checker_name, type_name)			\
	MEOW_DEFINE_NESTED_MEMBER_CHECKER(checker_name, type_name)				\
/**/

// the c++0x support enabled, use decltype for expression validation stuffs
//  compile with -std=c++0x to enable it
#if defined(__GXX_EXPERIMENTAL_CXX0X__)

#define MEOW_CXXOX_DEFINE_U_EXPRESSION_CHECKER(checker_name, U_expr)		\
template<class T>															\
struct checker_name															\
{																			\
	struct no_t {};															\
	template<class U> static decltype(U_expr)* test(void*);					\
	template<class U> static no_t test(...);								\
	typedef decltype(test<T>(NULL)) checked_t;								\
	enum { value = (sizeof(no_t) != sizeof(checked_t)) };					\
};																			\
/**/

#endif

#endif // MEOW_UTILITY__NESTED_TYPE_CHECKER_HPP_

