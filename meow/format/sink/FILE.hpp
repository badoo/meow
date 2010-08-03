////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_SINK_FILE_HPP_
#define MEOW_FORMAT_SINK_FILE_HPP_

#include <cstdio> 	// fwrite

#include <meow/api_call_error.hpp>
#include <meow/str_ref.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////
// a generic FILE as a sink (if you don't need to know anything about the file after writing)
//  can be use to simulate printf for example

	template<>
	struct sink_write<FILE*>
	{
		template<class CharT>
		static void call(FILE *& to_file, size_t total_len, string_ref<CharT const> const *slices, size_t n_slices)
		{
			for (size_t i = 0; i < n_slices; ++i)
			{
				string_ref<CharT const> const& slice = slices[i];
				size_t n = ::fwrite(slice.data(), slice.size() * sizeof(CharT), 1, to_file);
				if (1 != n) // n is the number of objects written, we always write 1
					throw meow::api_call_error("sink_write<FILE*>::call(): fwrite wrote less than requested");
			}
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct FILE_sink_t
	{
		FILE *f_;

		FILE_sink_t(FILE *f)
			: f_(f)
		{
		}

		template<class CharT>
		void write(size_t total_len, string_ref<CharT const> const *slices, size_t n_slices)
		{
			sink_write<FILE*>::call(f_, total_len, slices, n_slices);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}}} // namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_SINK_FILE_HPP_

