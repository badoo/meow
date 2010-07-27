////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_INSERTER_INTEGRAL_HPP_
#define MEOW_FORMAT_INSERTER_INTEGRAL_HPP_

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_integral.hpp>

#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>

#include <meow/format/detail/integer_to_string.hpp>
#include <meow/format/detail/integer_traits.hpp>
#include <meow/format/detail/radix_info.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
	namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

		// the wrapper used to print numbers in different
		//  radixes and capitalization
		//  nowhere near the qi/karma goodness, but compiles 100x faster
		template<class T, class RadixI>
		struct radix_number_wrapper_t
		{
			typedef RadixI radix_type;

			T value;
			radix_number_wrapper_t(T const& v) : value(v) {}
		};

////////////////////////////////////////////////////////////////////////////////////////////////
	} // namespace detail {
////////////////////////////////////////////////////////////////////////////////////////////////

#define MEOW_FORMAT__DEFINE_RADIX_WRAPPER_FUNCTION(fn_name, radix, radix_traits) 	\
	template<class T> 																\
	detail::radix_number_wrapper_t<T, detail::radix_traits<radix> > 				\
	fn_name(T const& value) { 														\
		return detail::radix_number_wrapper_t<T, detail::radix_traits<radix> >(value); 	\
	} 																				\
/**/

	MEOW_FORMAT__DEFINE_RADIX_WRAPPER_FUNCTION(as_hex, 16, radix_info_t);
	MEOW_FORMAT__DEFINE_RADIX_WRAPPER_FUNCTION(as_HEX, 16, radix_info_H_t);

	MEOW_FORMAT__DEFINE_RADIX_WRAPPER_FUNCTION(as_oct, 8, radix_info_t);
	MEOW_FORMAT__DEFINE_RADIX_WRAPPER_FUNCTION(as_bin, 2, radix_info_t);

#undef MEOW_FORMAT__DEFINE_RADIX_WRAPPER_FUNCTION

////////////////////////////////////////////////////////////////////////////////////////////////

	// regular integrals printing
	template<class T>
	struct type_tunnel<
			  T
			, typename boost::enable_if<boost::is_integral<T> >::type
			>
	{
		enum { radix = 10 };
		enum { buffer_size = detail::number_buffer_max_length<sizeof(T)*8, radix>::value };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(T v, buffer_t const& buf = buffer_t())
		{
			typedef detail::radix_info_t<radix> rinfo_t;
			char *b = detail::integer_to_string_ex<rinfo_t>(buf.get(), buf.size(), v);
			return str_ref(b, buf.end() - 1); // don't include the terminating-zero at end
		}
	};

	template<class T, class RI>
	struct type_tunnel<detail::radix_number_wrapper_t<T, RI> >
	{
		enum { buffer_size = detail::number_buffer_max_length<sizeof(T)*8, RI::radix>::value };
		typedef meow::tmp_buffer<buffer_size> buffer_t;

		static str_ref call(
				  detail::radix_number_wrapper_t<T, RI> const& v
				, buffer_t const& buf = buffer_t()
				)
		{
			char *b = detail::integer_to_string_ex<RI>(buf.get(), buf.size(), v.value);
			return str_ref(b, buf.get() + buf.size());
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_INSERTER_INTEGRAL_HPP_

