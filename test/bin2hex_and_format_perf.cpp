////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////
//
// g++ -O3 -ffast-math -g3 -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.47.0 -o bin2hex_and_format_perf bin2hex_and_format_perf.cpp
// 
// sample output when compiled with clang++ 3.1
//  antoxa@antoxa-mbp:~/_Dev/meow/test> ./bin2hex_and_format_perf
//  bin2hex_table_16 started: 1349193821.850068, n_iterations: 1000000
//  bin2hex_table_16 done: 1349193822.105130, elapsed: 0.255063
//  bin2hex_table_8 started: 1349193822.105150, n_iterations: 1000000
//  bin2hex_table_8 done: 1349193822.348021, elapsed: 0.242871
//  bin2hex_code started: 1349193822.348042, n_iterations: 1000000
//  bin2hex_code done: 1349193822.917355, elapsed: 0.569313
//  string started: 1349193822.917526, n_iterations: 1000000
//  string done: 1349193823.553606, elapsed: 0.636081
//  str_copy started: 1349193823.553624, n_iterations: 1000000
//  str_copy done: 1349193823.776004, elapsed: 0.222380
// 

#include <meow/stopwatch.hpp>
#include <meow/str_copy.hpp>

#include <meow/format/format.hpp>
#include <meow/format/format_to_string.hpp>
#include <meow/format/format_to_buffer_ref.hpp>
namespace ff = meow::format;

using meow::str_copy;
using meow::str_ref;
using meow::buffer_ref;

#include <type_traits>
#include <meow/convert/hex_to_from_bin.hpp>

inline char* bin2hex_table_16(unsigned char b, char* p)
{
	static char const codes_2[] = {
		'0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7',
		'0', '8', '0', '9', '0', 'a', '0', 'b', '0', 'c', '0', 'd', '0', 'e', '0', 'f',
		'1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7',
		'1', '8', '1', '9', '1', 'a', '1', 'b', '1', 'c', '1', 'd', '1', 'e', '1', 'f',
		'2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7',
		'2', '8', '2', '9', '2', 'a', '2', 'b', '2', 'c', '2', 'd', '2', 'e', '2', 'f',
		'3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7',
		'3', '8', '3', '9', '3', 'a', '3', 'b', '3', 'c', '3', 'd', '3', 'e', '3', 'f',
		'4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7',
		'4', '8', '4', '9', '4', 'a', '4', 'b', '4', 'c', '4', 'd', '4', 'e', '4', 'f',
		'5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7',
		'5', '8', '5', '9', '5', 'a', '5', 'b', '5', 'c', '5', 'd', '5', 'e', '5', 'f',
		'6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7',
		'6', '8', '6', '9', '6', 'a', '6', 'b', '6', 'c', '6', 'd', '6', 'e', '6', 'f',
		'7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7',
		'7', '8', '7', '9', '7', 'a', '7', 'b', '7', 'c', '7', 'd', '7', 'e', '7', 'f',
		'8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7',
		'8', '8', '8', '9', '8', 'a', '8', 'b', '8', 'c', '8', 'd', '8', 'e', '8', 'f',
		'9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7',
		'9', '8', '9', '9', '9', 'a', '9', 'b', '9', 'c', '9', 'd', '9', 'e', '9', 'f',
		'a', '0', 'a', '1', 'a', '2', 'a', '3', 'a', '4', 'a', '5', 'a', '6', 'a', '7',
		'a', '8', 'a', '9', 'a', 'a', 'a', 'b', 'a', 'c', 'a', 'd', 'a', 'e', 'a', 'f',
		'b', '0', 'b', '1', 'b', '2', 'b', '3', 'b', '4', 'b', '5', 'b', '6', 'b', '7',
		'b', '8', 'b', '9', 'b', 'a', 'b', 'b', 'b', 'c', 'b', 'd', 'b', 'e', 'b', 'f',
		'c', '0', 'c', '1', 'c', '2', 'c', '3', 'c', '4', 'c', '5', 'c', '6', 'c', '7',
		'c', '8', 'c', '9', 'c', 'a', 'c', 'b', 'c', 'c', 'c', 'd', 'c', 'e', 'c', 'f',
		'd', '0', 'd', '1', 'd', '2', 'd', '3', 'd', '4', 'd', '5', 'd', '6', 'd', '7',
		'd', '8', 'd', '9', 'd', 'a', 'd', 'b', 'd', 'c', 'd', 'd', 'd', 'e', 'd', 'f',
		'e', '0', 'e', '1', 'e', '2', 'e', '3', 'e', '4', 'e', '5', 'e', '6', 'e', '7',
		'e', '8', 'e', '9', 'e', 'a', 'e', 'b', 'e', 'c', 'e', 'd', 'e', 'e', 'e', 'f',
		'f', '0', 'f', '1', 'f', '2', 'f', '3', 'f', '4', 'f', '5', 'f', '6', 'f', '7',
		'f', '8', 'f', '9', 'f', 'a', 'f', 'b', 'f', 'c', 'f', 'd', 'f', 'e', 'f', 'f'
	};
	static uint16_t const *real_codes = reinterpret_cast<uint16_t const*>(codes_2);

	*(uint16_t*)p = real_codes[b];
	p += sizeof(uint16_t);

	return p;
}

inline char* bin2hex_table_8(unsigned char b, char *p)
{
	static char codes[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	*p++ = codes[b >> 4];
	*p++ = codes[b & 0xf];
	return p;
}

inline char* bin2hex_code(unsigned char b, char* p)
{
	unsigned char h = b >> 4;
	*p++ = '0' + h + (-(h > 9) & ('a' - '9' - 1));
	unsigned char l = b & 0xf;
	*p++ = '0' + l + (-(l > 9) & ('a' - '9' - 1));
	return p;
}

template<class Function>
inline char* copy_bin2hex(Function const& f, char const *b, char const *e, char *to)
{
	for (; b != e; ++b)
		to = f(*b, to);
	return to;
}

inline char* my_copy_bin2hex(char const *b, char const *e, char *to)
{
	return copy_bin2hex(&bin2hex_table_16, b, e, to);
}

template<class CharT>
inline
std::basic_string<typename std::remove_const<CharT>::type>
as_hex_string_string(meow::string_ref<CharT> const& s)
{
	std::basic_string<typename std::remove_const<CharT>::type> result(s.size() * 2, CharT());

	CharT *ee = copy_bin2hex(&bin2hex_code, s.begin(), s.end(), &*result.begin());
	assert(ee == &*result.end());

	return result;
}

template<class CharT>
inline
meow::string_copy<typename std::remove_const<CharT>::type>
as_hex_string_str_copy(meow::string_ref<CharT> const& s)
{
	meow::string_copy<typename std::remove_const<CharT>::type> result(s.size() * 2);

	CharT *ee = copy_bin2hex(&bin2hex_table_16, s.begin(), s.end(), &*result.begin());
	assert(ee == &*result.end());

	return result;
	//return move(result);
}

template<class Function>
void run_test(Function const& function, str_ref name, size_t n_iterations, str_ref const& str)
{
	char buf[str.size() * 2];

	meow::stopwatch_t sw;
	ff::fmt(stdout, "{0} started: {1}, n_iterations: {2}\n", name, sw.now(), n_iterations);

	for (size_t i = 0; i < n_iterations; ++i)
		copy_bin2hex(function, str.begin(), str.end(), buf);

	ff::fmt(stdout, "{0} done: {1}, elapsed: {2}\n", name, sw.now(), sw.stamp(), n_iterations);
}

template<class Function>
void run_format_test(Function const& function, str_ref name, size_t n_iterations, str_ref const& str)
{
	char buf[str.size() * 4];
	buffer_ref b(buf, sizeof(buf));

	meow::stopwatch_t sw;
	ff::fmt(stdout, "{0} started: {1}, n_iterations: {2}\n", name, sw.now(), n_iterations);

	for (size_t i = 0; i < n_iterations; ++i)
		ff::write_to_buffer(b, function(str));

	ff::fmt(stdout, "{0} done: {1}, elapsed: {2}\n", name, sw.now(), sw.stamp(), n_iterations);
}

int main(int argc, char **argv)
try
{
	size_t const n_iterations = 1 * 1000 * 1000;
	str_ref const str = "std::basic_STRING<TYPENAME BOOST::REMOVE_CONST<charT>::type> result(s.size() * 2, CharT());";

	run_test(&bin2hex_table_16, "bin2hex_table_16", n_iterations, str);
	run_test(&bin2hex_table_8, "bin2hex_table_8", n_iterations, str);
	run_test(&bin2hex_code, "bin2hex_code", n_iterations, str);

	run_format_test(&as_hex_string_string<char const>, "string", n_iterations, str);
	run_format_test(&as_hex_string_str_copy<char const>, "str_copy", n_iterations, str);

	return 0;
}
catch (std::exception const& e)
{
	ff::fmt(stdout, "exception: {0}\n", e.what());
	return 1;
}

