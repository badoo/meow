////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_SINK_CHAR_BUFFER_HPP_
#define MEOW_FORMAT_SINK_CHAR_BUFFER_HPP_

#include <cstdlib> 		// size_t
#include <stdexcept> 	// std::runtime_error

#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT>
	struct char_buffer_sink_t
	{
		typedef char_buffer_sink_t 			self_t;
		typedef CharT 						char_t;
		typedef string_ref<char_t const> 	string_slice_t;

	private:

		char_t *buf_;
		size_t buf_sz_, offset_;

	public:

		char_buffer_sink_t(char_t *b, size_t sz)
			: buf_(b)
			, buf_sz_(sz)
			, offset_(0)
		{
		}

		size_t size() const { return offset_; }
		size_t capacity() const { return buf_sz_; }

		void write(size_t total_len, string_slice_t const *slices, size_t n_slices)
		{
			if (capacity() < size() + total_len)
				throw std::runtime_error("char_buffer_sink_t::write() buffer will overflow");

			for (size_t i = 0; i < n_slices; ++i)
			{
				str_ref const& s = slices[i];
				std::memcpy(buf_ + offset_, (void*)s.begin(), s.size() * sizeof(char_t));
				offset_ += s.size();
			}

			BOOST_ASSERT(offset_ <= buf_sz_);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}}} // namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_SINK_CHAR_BUFFER_HPP_
