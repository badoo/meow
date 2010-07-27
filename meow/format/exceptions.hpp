////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT_FORMAT_EXCEPTIONS_HPP_
#define MEOW_FORMAT_FORMAT_EXCEPTIONS_HPP_

#include <exception>
#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct bad_format_string_t : public std::exception
	{
		str_ref fmt_;

		bad_format_string_t(str_ref fmt)
			: fmt_(fmt)
		{
		}

		virtual char const* what() const throw() { return "error parsing format string (check mismatched braces)"; }

		str_ref format_string() const throw() { return fmt_; }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct bad_argref_string_t : public std::exception
	{
		str_ref s_;

		bad_argref_string_t(str_ref s)
			: s_(s)
		{
		}

		virtual char const* what() const throw() { return "invalid argument reference: can't get number from string"; }

		str_ref argref_string() const throw() { return s_; }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct bad_argref_number_t : public std::exception
	{
		size_t n_;

		bad_argref_number_t(size_t n)
			: n_(n)
		{
		}

		virtual char const* what() const throw() { return "invalid argument reference: arg number is too big"; }

		size_t argref_number() const throw() { return n_; }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT_FORMAT_EXCEPTIONS_HPP_

