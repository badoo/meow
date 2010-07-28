////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_FORMAT_PARSER_HPP_
#define MEOW_FORMAT_FORMAT_PARSER_HPP_

#include <climits> // strtol depends on this according to the man page
#include <cstdlib> // for strtol

#include <meow/str_ref.hpp>
#include <meow/format/exceptions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class I>
	inline I iter_prior(I i) { return --i; }

	template<class I>
	inline I iter_next(I i) { return ++i; }

////////////////////////////////////////////////////////////////////////////////////////////////

	// return LONG_MAX if conversion failed
	// actual number if it succedes
	inline long str_ref_to_number(str_ref const& s)
	{
		char *endp = (char*)s.data();
		long result = ::strtol(s.begin(), &endp, 10);
		return (endp == s.end()) ? result : LONG_MAX;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	struct format_info_t
	{
		size_t total_length;
		size_t n_slices;
	};

	inline void push_slice(format_info_t& fi, str_ref new_slice, str_ref *slices, size_t n_slices)
	{
	//	printf("%s; new_slice: '%.*s'\n", __func__, new_slice.c_length(), new_slice.data());

		if (!new_slice)
			return;

		fi.total_length += new_slice.size();
		slices[fi.n_slices++] = new_slice;
		BOOST_ASSERT(fi.n_slices <= n_slices);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline format_info_t parse_format_expression(
			  str_ref fmt
			, str_ref *slices
			, size_t n_slices
			, str_ref *arg_slices
			, size_t n_arg_slices
		)
	{
		char const open_c = '{';
		char const close_c = '}';

		format_info_t result = {};

		typedef str_ref::iterator iterator_t;

		iterator_t curr = fmt.begin();
		iterator_t slice_start = curr;

		enum state_t { s_normal, s_argument };
		state_t state = s_normal;

		for (/**/; /**/; ++curr)
		{
			if (curr == fmt.end())
			{
				str_ref const slice_s = str_ref(slice_start, curr);
				push_slice(result, slice_s, slices, n_slices);
				break;
			}

			switch (state)
			{
				case s_normal:
					if (open_c == *curr)
					{
						iterator_t look_ahead = iter_next(curr);

						if (look_ahead == fmt.end())
							throw bad_format_string_t(fmt);

						if (open_c == *look_ahead)
						{
							++curr;
						}
						else
						{
							state = s_argument;
						}

						push_slice(result, str_ref(slice_start, curr), slices, n_slices);
						slice_start = iter_next(curr);
					}
					break;

				case s_argument:
					if (close_c == *curr)
					{
						str_ref const arg_s = str_ref(slice_start, curr);
						long const arg_n = str_ref_to_number(arg_s);

						if (LONG_MAX == arg_n)
							throw bad_argref_string_t(arg_s);

						if ((0 > arg_n) || (arg_n >= n_arg_slices))
							throw bad_argref_number_t(arg_n);

						push_slice(result, str_ref(arg_slices[arg_n]), slices, n_slices);

						slice_start = curr + 1;
						state = s_normal;
					}
					break;
			}
		}
	/*
		for (size_t i = 0; i < result.n_slices; ++i)
			printf("slice[%zu] = '%.*s'\n", i, slices[i].c_length(), slices[i].data());

		for (size_t i = 0; i < n_arg_slices; ++i)
			printf("arg_slice[%zu] = '%.*s'\n", i, arg_slices[i].c_length(), arg_slices[i].data());
	*/
		return result;
	}

	// need to think of some implementation here
	// current one is because "{N}" is 3 symbols long
	// and we get a good upper limit estimate from it
	inline size_t get_max_slices_for_format(str_ref const& fmt)
	{
		return fmt.size() / 3 + 1;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_FORMAT_PARSER_HPP_
