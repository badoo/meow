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
		, _max // <-- special element
		, _none = _max
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
		extra_items_macro 							\
	}; 												\
	enum { 											\
		  _total = BOOST_PP_SEQ_SIZE(enum_seq) 		\
	}; 												\
/**/

#define MEOW_SMART_ENUM_GEN_ENUM_NOTHING 	\
/**/

#define MEOW_SMART_ENUM_GEN_ENUM_EXTRAS 	\
	, _none 								\
/**/

#define MEOW_SMART_ENUM_GEN_ENUM_WITH_NONE(enum_seq) 		\
	MEOW_SMART_ENUM_GEN_ENUM(enum_seq, MEOW_SMART_ENUM_GEN_ENUM_EXTRAS) 	\
/**/

#define MEOW_SMART_ENUM_NAME_ITEM(z, n, seq) 			\
	BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_SEQ_ELEM(n, seq)) 	\
/**/

#define MEOW_SMART_ENUM_GEN_NAMES(ns_name, enum_seq) 		\
	static meow::str_ref names[] = { 						\
		BOOST_PP_ENUM( 										\
				  BOOST_PP_SEQ_SIZE(enum_seq) 				\
				, MEOW_SMART_ENUM_NAME_ITEM 				\
				, enum_seq 									\
			) 												\
	}; 														\
	enum { names_size = (sizeof(names) / sizeof(names[0])) }; 	\
	BOOST_STATIC_ASSERT(size_t(ns_name::_total) == size_t(names_size)); \
/**/

#define MEOW_SMART_ENUM_SWITCH_ITEM(r, ns_name, item) 	 	\
	case ns_name::BOOST_PP_TUPLE_ELEM(2, 0, item): 				\
		return BOOST_PP_TUPLE_ELEM(2, 1, item); 				\
/**/

#define MEOW_SMART_ENUM_GEN_FUNCTIONS(decl_prefix, ns_name, enum_seq)		\
	decl_prefix meow::str_ref enum_as_str_ref(ns_name::type t) { 			\
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
	decl_prefix char const* enum_as_string(ns_name::type t) { 	\
		return enum_as_str_ref(t).data(); 						\
	} 															\
	decl_prefix ns_name::type enum_from_str_ref(meow::str_ref s, ns_name::type not_found_res) { 	\
		for (size_t i = 0; i < detail::names_size; ++i) 		\
			if (detail::names[i] == s) 							\
				return (ns_name::type)i; 						\
		return not_found_res; 								\
	} 															\
/**/

#define MEOW_DEFINE_SMART_ENUM_I(ns_name, enum_seq, extra_items_macro_n) 	\
	namespace ns_name { 											\
		MEOW_SMART_ENUM_GEN_ENUM(enum_seq, extra_items_macro_n) 	\
		namespace detail { 											\
			MEOW_SMART_ENUM_GEN_NAMES(ns_name, enum_seq) 			\
		}; 															\
		MEOW_SMART_ENUM_GEN_FUNCTIONS(inline, ns_name, enum_seq) 	\
	} 																\
	typedef ns_name::type BOOST_PP_CAT(ns_name, _t); 				\
/**/


#define MEOW_DEFINE_SMART_ENUM(ns_name, enum_seq) 			\
	MEOW_DEFINE_SMART_ENUM_I(ns_name, enum_seq, MEOW_SMART_ENUM_GEN_ENUM_NOTHING) \
/**/

#define MEOW_DEFINE_SMART_ENUM_WITH_NONE(ns_name, enum_seq) 	\
	MEOW_DEFINE_SMART_ENUM_I(ns_name, enum_seq, MEOW_SMART_ENUM_GEN_ENUM_EXTRAS) \
/**/

#define MEOW_DEFINE_SMART_ENUM_STRUCT(ns_name, enum_seq) 	\
	struct ns_name { 											\
		MEOW_SMART_ENUM_GEN_ENUM(enum_seq, MEOW_SMART_ENUM_GEN_ENUM_NOTHING) 					\
		MEOW_SMART_ENUM_GEN_FUNCTIONS(inline static, ns_name, enum_seq) 	\
	}; 															\
	typedef ns_name::type BOOST_PP_CAT(ns_name, _t); 			\
/**/

#endif // MEOW__SMART_ENUM_HPP_

