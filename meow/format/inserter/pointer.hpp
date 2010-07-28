////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER__POINTER_HPP_
#define MEOW_FORMAT_INSERTER__POINTER_HPP_

#include <cstdlib>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_pointer.hpp>

#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>

#include <meow/format/detail/integer_to_string.hpp>
#include <meow/format/detail/integer_traits.hpp>
#include <meow/format/detail/radix_info.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	// all pointers printing
	template<class T>
	struct type_tunnel<T, typename boost::enable_if<boost::is_pointer<T> >::type>
	{
		enum { radix = 16 };
		enum { buffer_size = detail::number_buffer_max_length<sizeof(T)*8, radix>::value };
		typedef meow::tmp_buffer<buffer_size + 2> buffer_t;

		static str_ref call(T v, buffer_t const& buf = buffer_t())
		{
			typedef detail::radix_info_t<radix> rinfo_t;
			char *b = detail::integer_to_string_ex<rinfo_t>(buf.get(), buf.size(), uintptr_t(v));
			*--b = 'x';
			*--b = '0';
			return str_ref(b, buf.end() - 1);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER__POINTER_HPP_

