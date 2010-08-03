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
		, max // <-- special element
	};

	namespace detail {
		static char const* names[] = {
			  "unknown"
			, "add"
			, "delete"
			, "update"
		};
		BOOST_STATIC_ASSERT(request_type::max == (sizeof(names) / sizeof(names[0])));
	}

	inline char const* enum_as_string(type t)
	{
		BOOST_ASSERT(t < request_type::max);
		return detail::names[t];
	}
}
#endif

#define MEOW_SMART_ENUM_ENUM_ITEM(z, n, arr) 			\
	BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_SEQ_ELEM(n, arr)) 	\
/**/

#define MEOW_SMART_ENUM_GEN_ENUM(enum_seq) 		\
	enum type { 									\
		BOOST_PP_ENUM( 								\
				  BOOST_PP_SEQ_SIZE(enum_seq) 		\
				, MEOW_SMART_ENUM_ENUM_ITEM 		\
				, enum_seq 							\
			) 										\
	}; 												\
	enum { _total = BOOST_PP_SEQ_SIZE(enum_seq) }; 	\
/**/

#define MEOW_SMART_ENUM_NAME_ITEM(z, n, seq) 			\
	BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_SEQ_ELEM(n, seq)) 	\
/**/

#define MEOW_SMART_ENUM_GEN_NAMES(ns_name, enum_seq) 	\
	static char const* names[] = { 							\
		BOOST_PP_ENUM( 										\
				  BOOST_PP_SEQ_SIZE(enum_seq) 				\
				, MEOW_SMART_ENUM_NAME_ITEM 				\
				, enum_seq 									\
			) 												\
	}; 														\
	BOOST_STATIC_ASSERT(size_t(ns_name::_total) == (sizeof(names) / sizeof(names[0]))); \
/**/

#define MEOW_SMART_ENUM_SWITCH_ITEM(r, ns_name, item) 	 	\
	case ns_name::BOOST_PP_TUPLE_ELEM(2, 0, item): 				\
		return BOOST_PP_TUPLE_ELEM(2, 1, item); 				\
/**/

#define MEOW_SMART_ENUM_GEN_FUNCTIONS(decl_prefix, ns_name, enum_seq)		\
	decl_prefix char const* enum_as_string(ns_name::type t) { 			\
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
/**/

#define MEOW_DEFINE_SMART_ENUM(ns_name, enum_seq) 			\
	namespace ns_name { 										\
		MEOW_SMART_ENUM_GEN_ENUM(enum_seq) 					\
		namespace detail { 										\
			MEOW_SMART_ENUM_GEN_NAMES(ns_name, enum_seq) 	\
		}; 														\
		MEOW_SMART_ENUM_GEN_FUNCTIONS(inline, ns_name, enum_seq) 	\
	} 															\
	typedef ns_name::type BOOST_PP_CAT(ns_name, _t); 			\
/**/

#define MEOW_DEFINE_SMART_ENUM_STRUCT(ns_name, enum_seq) 	\
	struct ns_name { 											\
		MEOW_SMART_ENUM_GEN_ENUM(enum_seq) 					\
		MEOW_SMART_ENUM_GEN_FUNCTIONS(inline static, ns_name, enum_seq) 	\
	}; 															\
	typedef ns_name::type BOOST_PP_CAT(ns_name, _t); 			\
/**/

#endif // MEOW__SMART_ENUM_HPP_

