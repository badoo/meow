////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2006 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TMP_BUFFER_HPP_
#define MEOW_TMP_BUFFER_HPP_

#include <cstddef> // for size_t

///////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
///////////////////////////////////////////////////////////////////////////////////////////////
// temporary buffer
//  can be used as mutable object from a temporary parameter without const_cast<>
//  example
//  char* test_function(tmp_buffer<...> const& buf = tmp_buffer<...>())
//  {
//  	snprintf(buf.get(), buf.size(), ....);
//  	return buf.get();
//  }
//

	template<class T, std::size_t N>
	struct tmp_buffer_ex {
		typedef T			value_type;
		typedef value_type	buffer_type[N];

		enum { buffer_size = N };

		std::size_t size() const { return N; }
		buffer_type& get() const { return buf; }

		value_type* begin() const { return buf; }
		value_type* end() const { return begin() + size(); }

//		tmp_buffer_ex() { fprintf(stderr, "%s\n", __PRETTY_FUNCTION__); }
//		~tmp_buffer_ex() { fprintf(stderr, "%s\n", __PRETTY_FUNCTION__); }

	private:
		mutable buffer_type buf;
	};

///////////////////////////////////////////////////////////////////////////////////////////////

	template<std::size_t N>
	struct tmp_buffer : public tmp_buffer_ex<char, N> {};

	template<std::size_t N>
	struct w_tmp_buffer : public tmp_buffer_ex<wchar_t, N> {};

///////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
///////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TMP_BUFFER_HPP_


