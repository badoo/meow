////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_SINK_CHAR_BUFFER_HPP_
#define MEOW_FORMAT_SINK_CHAR_BUFFER_HPP_

#include <cstddef> 		// size_t
#include <cstring> 		// memcpy
#include <stdexcept> 	// std::runtime_error
#include <type_traits>  // std::is_trivially_copyable

#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class BufferT> // BufferT should have data(), size(), size_add(), capacity() and reset()
	struct plain_buffer_sink_impl_t : public BufferT
	{
		template<class... A>
		plain_buffer_sink_impl_t(A&&... args)
			: BufferT(std::forward<A>(args)...)
		{
		}

		static_assert(std::is_standard_layout<BufferT>::value, "cba supporting fancy types");

		buffer_ref used_part() const { return buffer_ref(this->buf(), this->size()); }
		buffer_ref free_part() const { return buffer_ref(this->buf() + this->size(), this->capacity()); }

		void write(size_t total_len, str_ref const *slices, size_t n_slices)
		{
			if (this->capacity() < this->size() + total_len)
				throw std::runtime_error("char_buffer_sink_t::write() buffer will overflow");

			for (size_t i = 0; i < n_slices; ++i)
			{
				str_ref const& s = slices[i];
				std::memcpy(this->buf() + this->size(), (void*)s.begin(), s.size());
				this->size_add(s.size());
			}

			assert(this->size() <= this->capacity());
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
// sinks priting to buffers they do not own

	template<class CharT>
	struct ref_buffer_t
	{
		CharT *buf_;
		size_t buf_sz_, offset_;

		ref_buffer_t(CharT *b, size_t buf_sz)
			: buf_(b)
			, buf_sz_(buf_sz)
			, offset_(0)
		{
		}

		CharT* buf() const { return buf_; }
		size_t size() const { return offset_; }
		void size_add(size_t sz) { offset_ += sz; }
		size_t capacity() const { return buf_sz_; }
		void reset() { offset_ = 0; }
	};

	template<class CharT>
	struct char_buffer_sink_impl_t
		: public plain_buffer_sink_impl_t<ref_buffer_t<CharT> >
	{
		using base_t = plain_buffer_sink_impl_t<ref_buffer_t<CharT> >;
		using char_t = CharT;

	public:

		char_buffer_sink_impl_t(char_t *b, size_t sz)
			: base_t(b, sz)
		{
		}

		char_buffer_sink_impl_t(string_ref<CharT> const& buf)
			: base_t(buf.data(), buf.size())
		{
		}
	};
	typedef char_buffer_sink_impl_t<char>     char_buffer_sink_t;
	typedef char_buffer_sink_impl_t<wchar_t>  wchar_buffer_sink_t;

////////////////////////////////////////////////////////////////////////////////////////////////
// simple stack type to return from functions by value

	template<class CharT, size_t N>
	struct stack_buffer_impl_t
	{
		size_t  length = 0;
		CharT   data[N];

		CharT* buf() { return data; }
		size_t size() const { return length; }
		void size_add(size_t sz) { length += sz; }
		size_t capacity() const { return N; }
		void reset() { length = 0; }

		operator string_ref<CharT>() const { return string_ref<CharT>(data, length); }
		operator string_ref<CharT const>() const { return string_ref<CharT const>(data, length); }
	};
	template<size_t N> using stack_buffer_t = stack_buffer_impl_t<char, N>;
	template<size_t N> using wstack_buffer_t = stack_buffer_impl_t<wchar_t, N>;
#if 0
	template<class CharT, size_t N>
	struct string_access<stack_buffer_impl_t<CharT, N> >
	{
		inline static str_ref call(stack_buffer_impl_t<CharT, N> const& b)
		{
			return 
			return str_ref((char const*)b.data, b.length * sizeof(CharT));
		}
	};
#endif
	template<class CharT, size_t N>
	struct stack_buffer_sink_impl_t
		: public plain_buffer_sink_impl_t<stack_buffer_impl_t<CharT, N> >
	{
	};
	template<size_t N> using stack_buffer_sink_t = stack_buffer_sink_impl_t<char, N>;
	template<size_t N> using wstack_buffer_sink_t = stack_buffer_sink_impl_t<wchar_t, N>;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_SINK_CHAR_BUFFER_HPP_

