////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__FORMAT_TO_STR_COPY_HPP_
#define MEOW_FORMAT__FORMAT_TO_STR_COPY_HPP_

#include <meow/str_copy.hpp>
#include "format_functions.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT, class Traits>
	struct sink_write<string_copy<CharT, Traits> >
	{
		static void call(
				  string_copy<CharT, Traits>& sink
				, size_t total_len
				, string_ref<CharT const> const *slices
				, size_t n_slices
				)
		{
			sink.reset_and_resize_to(total_len);

			char *to = sink.data();
			for (size_t i = 0; i < n_slices; ++i)
			{
				memcpy(to, slices[i].data(), slices[i].size());
				to += slices[i].size();
			}
		}
	};

	template<class F, class... A>
	inline str_copy fmt_str_copy(F const& f, A const&... args)
	{
		str_copy result;
		return fmt(result, f, args...);
	}

	template<class... A>
	inline str_copy write_str_copy(A const&... args)
	{
		str_copy result;
		return write(result, args...);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__FORMAT_TO_STR_COPY_HPP_

