////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2008 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__SMART_ENUM_HPP_
#define MEOW__SMART_ENUM_HPP_

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

#include <meow/str_ref.hpp>

// TODO: make enum value support and generate a switch on items,
// 			not a names array.
// 		this will require changing simple syntax

#if 0
MEOW_DEFINE_SMART_ENUM(request_type, 	((unknown, 	"unknown"))
										((add, 		"add"))
										((del, 		"delete"))
										((upd, 		"update"))
										);
generates

namespace request_type {
	enum type {
		  unknown
		, add
		, del
		, upd
		, _none // <-- special element if you asked for it with _WITH_NONE macro suffix
		// special names that are added for iteration and bounds
		, _min = unknown
		, _max = upd (NOT _none!)
	};

	enum { _total = 4 /* total_number_of_elements */ };

	namespace detail {
		static meow::str_ref names[] = {
			  "unknown"
			, "add"
			, "delete"
			, "update"
		};
		enum { names_size = (sizeof(names) / sizeof(names[0])) };
		BOOST_STATIC_ASSERT(request_type::max == names_size);
	}

	inline char const* enum_as_string(type t)
	{
		BOOST_ASSERT(t < request_type::max);
		return detail::names[t];
	}

	inline meow::str_ref enum_as_str_ref(type t) // returns name by enum
	type enum_from_str_ref(meow::str_ref s); // returns enum by it's name

	// another macro that iterates over the enum
	MEOW_SMART_ENUM_FOREACH(request_type, loop_variable)
	{
	}
}
#endif

#define MEOW_SMART_ENUM_ENUM_ITEM(z, n, arr) 			\
	BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_SEQ_ELEM(n, arr)) 	\
/**/

#define MEOW_SMART_ENUM_GEN_ENUM(enum_seq, extra_items_macro) 		\
	enum type { 									\
		BOOST_PP_ENUM( 								\
				  BOOST_PP_SEQ_SIZE(enum_seq) 		\
				, MEOW_SMART_ENUM_ENUM_ITEM 		\
				, enum_seq 							\
			) 										\
		extra_items_macro() 						\
		, _min = MEOW_SMART_ENUM_ENUM_ITEM(_, 0, enum_seq) \
		, _max = MEOW_SMART_ENUM_ENUM_ITEM(_, BOOST_PP_DEC(BOOST_PP_SEQ_SIZE(enum_seq)), enum_seq) \
	}; 												\
	enum { 											\
		  _total = BOOST_PP_SEQ_SIZE(enum_seq) 		\
	}; 												\
/**/

#define MEOW_SMART_ENUM_GEN_ENUM_NOTHING(_) 	\
/**/

#define MEOW_SMART_ENUM_GEN_ENUM_EXTRAS(_) 	\
	, _none 								\
/**/

#define MEOW_SMART_ENUM_GEN_ENUM_WITH_NONE(enum_seq) 		\
	MEOW_SMART_ENUM_GEN_ENUM(enum_seq, MEOW_SMART_ENUM_GEN_ENUM_EXTRAS) 	\
/**/

#define MEOW_SMART_ENUM_NAME_ITEM(z, n, seq) 					\
	BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_SEQ_ELEM(n, seq)) 		\
/**/

#define MEOW_SMART_ENUM_SWITCH_ITEM(r, ns_name, item) 	 		\
	case BOOST_PP_TUPLE_ELEM(2, 0, item): 						\
		return BOOST_PP_TUPLE_ELEM(2, 1, item); 				\
/**/

#define MEOW_SMART_ENUM_FROM_IF_ITEM(r, ns_name, item) 			\
	if (meow::ref_lit(BOOST_PP_TUPLE_ELEM(2, 1, item)) == s) 	\
		return BOOST_PP_TUPLE_ELEM(2, 0, item); 				\
/**/

#define MEOW_SMART_ENUM_GEN_FUNCTIONS(decl_prefix, ns_name, enum_seq, has_none)		\
	decl_prefix meow::str_ref enum_as_str_ref(type t) { 			\
		switch (t) { 											\
			BOOST_PP_SEQ_FOR_EACH( 								\
					  MEOW_SMART_ENUM_SWITCH_ITEM 			\
					, ns_name 									\
					, enum_seq 									\
				) 												\
			default: 											\
				BOOST_ASSERT(!"invalid enum value"); 			\
		} 														\
	} 															\
	decl_prefix char const* enum_as_string(type t) { 	\
		return enum_as_str_ref(t).data(); 						\
	} 															\
	decl_prefix type enum_from_str_ref( 				\
			  meow::str_ref s 									\
			, type not_found_res BOOST_PP_IF(has_none, = ns_name::_none, )	\
		) 														\
	{ 															\
		BOOST_PP_SEQ_FOR_EACH(MEOW_SMART_ENUM_FROM_IF_ITEM, ns_name, enum_seq) \
		return not_found_res; 									\
	} 															\
/**/

#define MEOW_EXTRAS_ELEM(e, n) BOOST_PP_TUPLE_ELEM(2, n, e)

#define MEOW_DEFINE_SMART_ENUM_I(ns_name, enum_seq, extras) 		\
	namespace ns_name { 											\
		MEOW_SMART_ENUM_GEN_ENUM(enum_seq, MEOW_EXTRAS_ELEM(extras, 1)) 	\
		MEOW_SMART_ENUM_GEN_FUNCTIONS(inline, ns_name, enum_seq, MEOW_EXTRAS_ELEM(extras, 0)) 	\
	} 																\
	typedef ns_name::type BOOST_PP_CAT(ns_name, _t); 				\
/**/


#define MEOW_DEFINE_SMART_ENUM(ns_name, enum_seq) 			\
	MEOW_DEFINE_SMART_ENUM_I(ns_name, enum_seq, (0, MEOW_SMART_ENUM_GEN_ENUM_NOTHING)) \
/**/

#define MEOW_DEFINE_SMART_ENUM_WITH_NONE(ns_name, enum_seq) 	\
	MEOW_DEFINE_SMART_ENUM_I(ns_name, enum_seq, (1, MEOW_SMART_ENUM_GEN_ENUM_EXTRAS)) \
/**/

#define MEOW_DEFINE_SMART_ENUM_STRUCT_I(ns_name, enum_seq) 						\
	struct ns_name { 															\
		MEOW_SMART_ENUM_GEN_ENUM(enum_seq, MEOW_SMART_ENUM_GEN_ENUM_NOTHING) 	\
		MEOW_SMART_ENUM_GEN_FUNCTIONS(inline static, ns_name, enum_seq, 0) 		\
	}; 																			\
/**/

#define MEOW_DEFINE_SMART_ENUM_STRUCT(ns_name, enum_seq) 		\
	MEOW_DEFINE_SMART_ENUM_STRUCT_I(ns_name, enum_seq)			\
	typedef ns_name::type BOOST_PP_CAT(ns_name, _t);			\
/**/

// same as above, but use from inside template classes
#define MEOW_DEFINE_SMART_ENUM_STRUCT_T(ns_name, enum_seq) 		\
	MEOW_DEFINE_SMART_ENUM_STRUCT_I(ns_name, enum_seq)			\
	typedef typename ns_name::type BOOST_PP_CAT(ns_name, _t);	\
/**/

#define MEOW_SMART_ENUM_FOREACH(seq, i) 					\
	for (seq##_t i = seq::_min; i <= seq::_max; i = (seq##_t)((int)i + 1)) 	\
/**/

#endif // MEOW__SMART_ENUM_HPP_

