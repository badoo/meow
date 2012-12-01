////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_SINK_FD_HPP_
#define MEOW_FORMAT_SINK_FD_HPP_

#include <fcntl.h>    // open
#include <sys/uio.h>  // writev

#include <meow/api_call_error.hpp>
#include <meow/str_ref.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////
// a generic fd as a sink (if you don't need to know anything about the file after writing)
//  can be use to simulate printf for example

	struct fd_sink_t
	{
		int fd;
		fd_sink_t(int f) : fd(f) {}
	};

	template<>
	struct sink_write<fd_sink_t>
	{
		template<class CharT>
		static void call(fd_sink_t& sink, size_t total_len, string_ref<CharT const> const *slices, size_t n_slices)
		{
			char buf[total_len];
			size_t offset = 0;
			for (size_t i = 0; i < n_slices; ++i)
			{
				memcpy(buf + offset, slices[i].data(), slices[i].size() * sizeof(CharT));
				offset += slices[i].size() * sizeof(CharT);
			}

			for (offset = 0; offset < total_len; /**/)
			{
				int n = ::write(sink.fd, buf + offset, total_len - offset);
				if (-1 == n)
					throw meow::api_call_error("write");
				if (0 == n)
					throw std::runtime_error("write() returned 0");

				offset += n;
			}

			return;

			typedef struct iovec os_iovec_t;

			os_iovec_t iov[n_slices];

			for (size_t i = 0; i < n_slices; ++i)
			{
				str_ref const& s = slices[i];
				iov[i].iov_base = (void*)s.data();
				iov[i].iov_len = s.size() * sizeof(CharT);
			}

			for (os_iovec_t *b = iov, *const e = iov + n_slices; b < e; /**/)
			{
				ssize_t r = ::writev(sink.fd, b, e - b);
				if (-1 == r)
					throw meow::api_call_error("writev");

				if (0 == r)
					throw std::runtime_error("writev() returned 0");

				for (size_t len = r; len > 0; /**/)
				{
					if (b->iov_len <= len /* && b < e */)
					{
						len -= b->iov_len;
						++b;
					}
					else
					{
						b->iov_base = (char*)b->iov_base + len;
						b->iov_len -= len;
						break;
					}
				}
			}
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}}} // namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_SINK_FD_HPP_

