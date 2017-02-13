////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_FORMAT_PARSER_HPP_
#define MEOW_FORMAT_FORMAT_PARSER_HPP_

#include <cassert>
#include <cstring> // std::memchr

#include <meow/str_ref.hpp>
#include <meow/format/exceptions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct format_info_t
	{
		size_t total_length = 0;
		size_t n_slices     = 0;
	};

	inline void push_slice(format_info_t& fi, str_ref const& new_slice, str_ref *slices, size_t n_slices)
	{
	//	printf("%s; new_slice: '%.*s'\n", __func__, new_slice.c_length(), new_slice.data());

		if (!new_slice)
			return;

		fi.total_length += new_slice.size();
		slices[fi.n_slices++] = new_slice;
		assert(fi.n_slices <= n_slices);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline format_info_t parse_format_expression(
			  str_ref const& fmt
			, str_ref *slices
			, size_t n_slices
			, str_ref const *arg_slices
			, size_t n_arg_slices
		)
	{
		static char const open_c = '{';
		static char const close_c = '}';

		format_info_t result = {};

		char const *head = fmt.begin();
		char const *hend = fmt.end();

		while (head != hend)
		{
			char const *abegin = (char const*)std::memchr(head, open_c, hend - head);
			if (NULL == abegin)
			{
				push_slice(result, str_ref(head, hend), slices, n_slices);
				break;
			}

			if (open_c == *++abegin) // this is not an arg
			{
				push_slice(result, str_ref(head, abegin), slices, n_slices);
				head = abegin + 1;
				continue;
			}
			else // this is an arg
			{
				push_slice(result, str_ref(head, abegin - 1), slices, n_slices);
				head = abegin;

				unsigned int arg_n = 0;
				if (__builtin_expect(std::isdigit(*head), 1))
				{
					arg_n = *head++ - '0';

					for (; head != hend && std::isdigit(*head); ++head)
					{
						arg_n *= 10;
						arg_n += *head - '0';
					}
				}

				if (__builtin_expect(head == hend, 0))
					throw bad_format_string_t(fmt);

				if (__builtin_expect(close_c != *head++, 0))
					throw bad_argref_string_t(fmt);

				if (__builtin_expect(arg_n >= n_arg_slices, 0))
				{
					push_slice(result, ref_lit("<!ARG:"), slices, n_slices);
					push_slice(result, str_ref(abegin, head-1), slices, n_slices);
					push_slice(result, ref_lit(">"), slices, n_slices);
					//throw bad_argref_number_t(arg_n);
				}
				else
					push_slice(result, arg_slices[arg_n], slices, n_slices);
			}
		}

		return result;
	}

	// need to think of some implementation here
	// current one is because "{N}" is 3 symbols long
	// and we get a good upper limit estimate from it
	//  division by 2 and not 3 because we can have stuff between the markers
	inline size_t get_max_slices_for_format(str_ref const& fmt)
	{
		static size_t const min_n_slices = 8;

		size_t const n_slices = fmt.size() / 2 + 1;
		return (n_slices < min_n_slices) ? min_n_slices : n_slices;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_FORMAT_PARSER_HPP_
