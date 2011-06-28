////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__GENERIC_NEW_HPP_
#define MEOW__GENERIC_NEW_HPP_

#include <meow/move_ptr/static_move_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<
		  class T
		, class ResultT = boost::static_move_ptr<T> // client must include the header
	>
	struct generic_new_t
	{
		typedef ResultT result_type;

		result_type operator()() const
		{ return result_type(new T()); }

		template<class A0>
		result_type operator()(A0 const& a0) const
		{ return result_type(new T(a0)); }

		template<class A0, class A1>
		result_type operator()(A0 const& a0, A1 const& a1) const
		{ return result_type(new T(a0, a1)); }

		template<class A0, class A1, class A2>
		result_type operator()(A0 const& a0, A1 const& a1, A2 const& a2) const
		{ return result_type(new T(a0, a1, a2)); }

		template<class A0, class A1, class A2, class A3>
		result_type operator()(A0 const& a0, A1 const& a1, A2 const& a2, A3 const& a3) const
		{ return result_type(new T(a0, a1, a2, a3)); }

		template<class A0, class A1, class A2, class A3, class A4>
		result_type operator()(A0 const& a0, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4) const
		{ return result_type(new T(a0, a1, a2, a3, a4)); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__GENERIC_NEW_HPP_

