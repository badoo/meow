////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 fdm=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_MAPPING_DETAIL__ASSIGN_HPP_
#define MEOW_MAPPING_DETAIL__ASSIGN_HPP_

#include <meow/convert/whatever_cast.hpp>
#include <meow/convert/whatever_from_str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct default_assign_t
	{
		typedef void result_type;

		template<class To, class From>
		void operator()(To& to, From const& from) const
		{
			static whatever_cast<To, From> caster_;
			caster_(to, from);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class ValueT>
	struct specific_value_assign_t
	{
		typedef void result_type;

		explicit specific_value_assign_t(ValueT const& v)
			: value_(v)
		{
		}

		template<class To, class From>
		void operator()(To& to, From const&) const
		{
			caster_(to, value_);
		}

	private:
		whatever_cast<To, ValueT> caster_;
		ValueT const value_;
	};

	template<class T>
	specific_value_assign_t<T> assign_specific(T const& v)
	{
		return specific_value_assign_t<T>(v);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_MAPPING_DETAIL__ASSIGN_HPP_

