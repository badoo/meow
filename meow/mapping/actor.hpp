////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 fdm=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_MAPPING__ACTOR_HPP_
#define MEOW_MAPPING__ACTOR_HPP_

#include <cstddef> // for size_t + let users use offsetof macro without including it manualy

#include <boost/assert.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/remove_reference.hpp>

#include <meow/str_ref.hpp>

#include "detail/assign.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////
// obj casters

// simple noop modifier and caster
	struct noop_modifier_t
	{
		template<class U>
		struct result { typedef typename boost::add_reference<U>::type type; };

		template<class T>
		typename result<T>::type operator()(T& ctx) const { return ctx; }
	};

// just trying to static_cast to a given type
	template<class T>
	struct static_caster_t
	{
		template<class U>
		struct result { typedef typename boost::add_reference<T>::type type; };

		template<class U>
		typename result<T>::type operator()(U& ctx) const
		{
			return static_cast<typename result<T>::type>(ctx);
		}
	};

// selecting specific member of some structure
	template<class MT, class T>
	struct select_member_t
	{
		typedef MT T::*member_ptr_t;
		member_ptr_t member_p;

		template<class U>
		struct result { typedef typename boost::add_reference<MT>::type type; };

		explicit select_member_t(member_ptr_t p) : member_p(p) {}
		typename result<T>::type operator()(T& obj) const { return obj.*member_p; }
	};

	template<class MT, class T>
	select_member_t<MT, T> member(MT T::* p) { return select_member_t<MT, T>(p); }

// interpreting address offsets as typed values
	template<class TargetT>
	struct typed_offset_t
	{
		int offset_;

		template<class U>
		struct result { typedef typename boost::add_reference<TargetT>::type type; };

		explicit typed_offset_t(int offset) : offset_(offset) {}

		template<class T>
		typename result<T>::type operator()(T& obj) const
		{
			char *p = reinterpret_cast<char*>(&obj) + offset_;

			// assert that alignment restrictions for target type are met
			//  don't care about losing precision when converting ptr -> size_t
			size_t ptr_as_int = reinterpret_cast<size_t>(p);
			BOOST_ASSERT("target pointer is aligned for TargetT" && 0 == (ptr_as_int % boost::alignment_of<TargetT>::value));

			return *reinterpret_cast<TargetT*>(p);
		}
	};

// appends new default constructed item to a sequence
//  and returns a reference to it

	struct seq_opbase_t
	{
		template<class U>
		struct result {
			typedef typename boost::remove_reference<U>::type::value_type obj_type;
			typedef obj_type& type;
		};
	};

	struct seq_append_t : public seq_opbase_t
	{
		template<class SequenceT>
		typename result<SequenceT>::type operator()(SequenceT& seq) const
		{
			seq.insert(seq.end(), typename result<SequenceT>::obj_type());
			return seq.back();
		}
	};

	struct seq_push_back_t : public seq_opbase_t
	{
		template<class SequenceT>
		typename result<SequenceT>::type operator()(SequenceT& seq) const
		{
			seq.push_back(typename result<SequenceT>::obj_type());
			return seq.back();
		}
	};

	struct seq_prepend_t : public seq_opbase_t
	{
		template<class SequenceT>
		typename result<SequenceT>::type operator()(SequenceT& seq) const
		{
			seq.insert(seq.begin(), typename result<SequenceT>::obj_type());
			return seq.front();
		}
	};

	struct seq_push_front_t : public seq_opbase_t
	{
		template<class SequenceT>
		typename result<SequenceT>::type operator()(SequenceT& seq) const
		{
			seq.push_front(typename result<SequenceT>::obj_type());
			return seq.front();
		}
	};

// combines 2 casters in a form of outer_caster(inner_caster)
	template<class OuterCast, class InnerCast>
	struct composite_caster_t
	{
		template<class U>
		struct result {
			typedef typename InnerCast::template result<U>::type inner_result_type;
			typedef typename boost::remove_reference<inner_result_type>::type inner_good_result;
			typedef typename OuterCast::template result<inner_good_result>::type type;
		};

		composite_caster_t(OuterCast const& o, InnerCast const& i)
			: outer_(o)
			, inner_(i)
		{
		}
		
		template<class ContextT>
		typename result<ContextT>::type operator()(ContextT& ctx) const
		{
			return outer_(inner_(ctx));
		}

	private:
		OuterCast const outer_;
		InnerCast const inner_;
	};

	template<class OuterCast, class InnerCast>
	composite_caster_t<OuterCast, InnerCast> composite_cast(OuterCast const& o, InnerCast const& i) {
		return composite_caster_t<OuterCast, InnerCast>(o, i);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
// handlers

// combiner for extractor + a handler, used by 'short form' calls, like set_value(&some_t::some_member)
	template<class Extractor, class Function>
	struct extract_and_call_t
	{
		Extractor const extractor_;
		Function const function_;

		explicit extract_and_call_t(Extractor const& e, Function const& f = Function())
			: extractor_(e)
			, function_(f)
		{
		}

		template<class ContextT, class ValueT>
		void operator()(ContextT& ctx, ValueT const& value) const
		{
			function_(extractor_(ctx), value);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
// convenience functions

	inline noop_modifier_t dont_cast() { return noop_modifier_t(); }

	template<class T>
	inline static_caster_t<T> static_cast_to() { return static_caster_t<T>(); }

	template<class MT, class T>
	inline select_member_t<MT, T> do_cast(MT T::* p) { return member(p); }

	template<class MT, class T>
	extract_and_call_t<select_member_t<MT, T>, default_assign_t>
	set_value(MT T::* p)
	{
		typedef extract_and_call_t<select_member_t<MT,T>, default_assign_t> result_type;
		return result_type(member(p));
	}

	template<class OuterMT, class InnerMT, class T>
	struct set_value_2_result
	{
		typedef extract_and_call_t<
			composite_caster_t<
				  select_member_t<OuterMT, InnerMT>
				, select_member_t<InnerMT, T>
			>
			, default_assign_t
		> type;
	};

	template<class OuterMT, class InnerMT, class T>
	typename set_value_2_result<OuterMT, InnerMT, T>::type
	set_value(InnerMT T:: *inner_p, OuterMT InnerMT:: *outer_p)
	{
		typedef typename set_value_2_result<OuterMT, InnerMT, T>::type result_type;
		return result_type(composite_cast(member(outer_p), member(inner_p)), default_assign_t());
	}

	template<class TargetT>
	extract_and_call_t<typed_offset_t<TargetT>, default_assign_t>
	set_offset(int offset)
	{
		typedef extract_and_call_t<typed_offset_t<TargetT>, default_assign_t> result_type;
		return result_type(typed_offset_t<TargetT>(offset));
	}

	template<class MT, class T>
	extract_and_call_t<select_member_t<MT, T>, specific_value_assign_t<MT> >
	just_assign(MT T:: *p, MT const& value)
	{
		typedef extract_and_call_t<select_member_t<MT, T>, specific_value_assign_t<MT> > result_type;
		return result_type(member(p), assign_specific(value));
	}

#define DEFINE_JUST_ASSIGN_SPECIALIZATION(spec_func_name, just_value)				\
	template<class MT, class T>														\
	extract_and_call_t<select_member_t<MT, T>, specific_value_assign_t<MT> >		\
	spec_func_name(MT T:: *p)	{ 													\
		return just_assign(p, just_value);											\
	}																				\
// ENDMACRO: DEFINE_JUST_ASSIGN_SPECIALIZATION

	DEFINE_JUST_ASSIGN_SPECIALIZATION(just_assign_true, true)
	DEFINE_JUST_ASSIGN_SPECIALIZATION(just_assign_false, false)
	DEFINE_JUST_ASSIGN_SPECIALIZATION(just_assign_zero, 0)

#undef DEFINE_JUST_ASSIGN_SPECIALIZATION

#define MEOW_MAPPING_SEQUENCE_MEMBER_CAST_TYPE(seq_op)	\
	composite_caster_t<seq_op, select_member_t<MT, T> > 	\
/**/

#define MEOW_MAPPING_SEQUENCE_MEMBER_OP_TYPE(seq_op, handler_fn)		\
	extract_and_call_t<													\
		  MEOW_MAPPING_SEQUENCE_MEMBER_CAST_TYPE(seq_op)				\
		, handler_fn													\
	> 																	\
/**/

// exported macro
#define MEOW_MAPPING_DEFINE_SEQUENCE_OP(name, seq_op, handler_fn)			\
	template<class MT, class T>													\
	MEOW_MAPPING_SEQUENCE_MEMBER_OP_TYPE(seq_op, handler_fn)					\
	name(MT T::* p) {															\
		typedef MEOW_MAPPING_SEQUENCE_MEMBER_OP_TYPE(seq_op, handler_fn) result_type;	\
		return result_type(composite_cast(seq_op(), member(p)), handler_fn());	\
	} 																			\
/**/

	MEOW_MAPPING_DEFINE_SEQUENCE_OP(seq_append, seq_append_t, default_assign_t);
	MEOW_MAPPING_DEFINE_SEQUENCE_OP(seq_push_back, seq_push_back_t, default_assign_t);
	MEOW_MAPPING_DEFINE_SEQUENCE_OP(seq_prepend, seq_prepend_t, default_assign_t);
	MEOW_MAPPING_DEFINE_SEQUENCE_OP(seq_push_front, seq_push_front_t, default_assign_t);

#define DEFINE_SEQUENCE_MEMBER_CAST(name, seq_op)		\
	template<class MT, class T>							\
	MEOW_MAPPING_SEQUENCE_MEMBER_CAST_TYPE(seq_op)	\
	name(MT T:: *p) {									\
		return composite_cast(seq_op(), member(p));		\
	} 													\
/**/

	DEFINE_SEQUENCE_MEMBER_CAST(new_back_of, seq_append_t);
	DEFINE_SEQUENCE_MEMBER_CAST(new_front_of, seq_prepend_t);

#undef DEFINE_SEQUENCE_MEMBER_CAST

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_MAPPING__ACTOR_HPP_


