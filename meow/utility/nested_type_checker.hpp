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

#define MEOW_DEFINE_NESTED_TYPE_CHECKER(checker_name, type_name)			\
	template<class T>														\
	struct checker_name														\
	{																		\
		struct yes_type {};													\
		struct no_type { yes_type dummy[2]; };								\
																			\
		template<class U> static yes_type test(typename U::type_name *);	\
		template<class U> static no_type test(...);							\
																			\
		enum { value = (sizeof(test<T>(0)) == sizeof(yes_type)) };			\
	};																		\
/* ENDMACRO: MEOW_DEFINE_NESTED_TYPE_CHECKER */

#endif // MEOW_UTILITY__NESTED_TYPE_CHECKER_HPP_

