////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__FORMAT_FUNCTIONS_HPP_
#define MEOW_FORMAT__FORMAT_FUNCTIONS_HPP_

#include <alloca.h>
#include <meow/str_ref.hpp>
#include <meow/format/metafunctions.hpp>
#include <meow/format/format_parser.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class S, class F, class... A>
	inline S& fmt_tunnel(S& sink, F const& fmt, A const&... args)
	{
		size_t const max_slices = get_max_slices_for_format(fmt);
		str_ref *slices = (str_ref*)alloca(max_slices * sizeof(str_ref));

		static size_t const n_arg_slices = sizeof...(A);
		str_ref const arg_slices[] = { string_access<A>::call(args)... };

		format_info_t const fi = parse_format_expression(
									  string_access<F>::call(fmt)
									, slices, max_slices
									, arg_slices, n_arg_slices
									);
		write_to_sink(sink, fi.total_length, slices, fi.n_slices);
		return sink;
	}

	template<class S, class F, class... A>
	inline S& fmt(S& sink, F const& fmt, A const&... args)
	{
		return fmt_tunnel(
					  sink
					, type_tunnel<F>::call(fmt)
					, type_tunnel<A>::call(args)...);
	}

	template<class S, class... A>
	inline S& write_tunnel(S& sink, A const&... args)
	{
		static size_t const n_slices = sizeof...(A);
		str_ref slices[] = { string_access<A>::call(args)... };

		size_t total_length = 0;
		for (str_ref const& s : slices)
			total_length += s.size();

		write_to_sink(sink, total_length, slices, n_slices);
		return sink;
	}

	template<class S, class... A>
	inline S& write(S& sink, A const&... args)
	{
		return write_tunnel(sink, type_tunnel<A>::call(args)...);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_FUNCTIONS_HPP_

