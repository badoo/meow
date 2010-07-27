////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_SINK_FILE_HPP_
#define MEOW_FORMAT_SINK_FILE_HPP_

#include <cstdio>
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct FILE_sink_t
	{
		FILE *f_;

		FILE_sink_t(FILE *f)
			: f_(f)
		{
		}

		void write(size_t total_len, str_ref const *slices, size_t n_slices)
		{
			for (size_t i = 0; i < n_slices; ++i)
			{
				str_ref const& slice = slices[i];
				size_t n = ::fwrite(slice.data(), slice.size(), 1, f_);
				if (1 != n) // n is the number of objects written, we always write 1
					throw std::runtime_error("FILE_sink_t::write(): fwrite wrote less than we requested");
			}
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}}} // namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_SINK_FILE_HPP_

