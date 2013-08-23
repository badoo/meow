////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 fdm=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_SINK__BUFFER_HPP_
#define MEOW_FORMAT_SINK__BUFFER_HPP_

#include <meow/buffer.hpp>
#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	struct sink_write<buffer_impl_t<CharT> >
	{
		static void call(
					  buffer_impl_t<CharT>& buf
					, size_t total_len
					, meow::string_ref<CharT const> const* slices
					, size_t n_slices
					)
		{
			if (buf.free_size() < total_len)
				buf.resize_to(buf.used_size() + total_len + 1);

			for (size_t i = 0; i < n_slices; ++i)
				copy_to_buffer(buf, slices[i].data(), slices[i].size() * sizeof(CharT));
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	struct buffer_sink_impl_t
	{
		typedef buffer_sink_impl_t 		self_t;
		typedef buffer_impl_t<CharT> 	buf_t;

		buffer_move_ptr buf_;

		explicit buffer_sink_impl_t(size_t initial_sz)
			: buf_(new buf_t(initial_sz))
		{
		}

		buffer_move_ptr grab_buffer() { return move(buf_); }

		void write(size_t total_len, meow::string_ref<CharT const> const* slices, size_t n_slices)
		{
			sink_write<buf_t>::call(*buf_, total_len, slices, n_slices);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef buffer_sink_impl_t<char> 		buffer_sink_t;
	typedef buffer_sink_impl_t<wchar_t> 	w_buffer_sink_t;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_SINK__BUFFER_HPP_

