////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2006 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_BITFIELD_UNION_HPP_
#define MEOW_BITFIELD_UNION_HPP_

#include <boost/static_assert.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class BitT, class IntT = unsigned long>
	struct bitfield_union
	{
		typedef bitfield_union	self_type;
		typedef BitT			bitfield_type;
		typedef IntT			integer_type;

		// NOTE: maybe we should assert that integer_type is really an integer
		//  but this is not desirable, because we want to be able to
		//  pass something like struct big_int_t { uint32_t data[4]; };

		BOOST_STATIC_ASSERT(sizeof(bitfield_type) <= sizeof(integer_type));

		union {
			bitfield_type bits;
			integer_type value;
		};

		// ctors
		bitfield_union() : value() {}
		explicit bitfield_union(integer_type v) : value(v) {}
		explicit bitfield_union(bitfield_type const& bf) : bits(bf) {}

		// assign
		integer_type operator=(integer_type v) { return value = v; }
		bitfield_type& operator=(bitfield_type const& bf) { return bits = bf; }

		// convert to integer
		operator integer_type() const { return value; }
		operator integer_type&() { return value; }

		// convert to bitfields structure
		operator bitfield_type&() { return bits; }
		operator bitfield_type const&() const { return bits; }

		// access
		bitfield_type* operator->() { return &bits; }
		bitfield_type const* operator->() const { return &bits; }

		// more accessors
		friend integer_type as_integer(self_type const& self) { return self; }
		friend bitfield_type& as_bitfield(self_type& self) { return self; }
		friend bitfield_type const& as_bitfield(self_type const& self) { return self; }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_BITFIELD_UNION_HPP_

