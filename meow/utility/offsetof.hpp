////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// Copyright(c) 2009+ Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_UTILITY__OFFSETOF_HPP_
#define MEOW_UTILITY__OFFSETOF_HPP_

////////////////////////////////////////////////////////////////////////////////////////////////
// simple macros for member offsets and arithmetic utils

#define MEOW_OFFSETOF(class_name, member_name) \
	ptrdiff_t(((char*)&((class_name*)1)->member_name) - 1)

#define MEOW_SELF_FROM_MEMBER(class_name, member_name, member_ptr) \
	((class_name*)((char*)member_ptr - MEOW_OFFSETOF(class_name, member_name)))

#define MEOW_MEMBER_PTR_AT_OFFSET(class_ptr, member_type, member_offset) \
	((member_type*)((char*)class_ptr + member_offset))

////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_UTILITY__OFFSETOF_HPP_

