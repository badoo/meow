////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_ERROR_HPP_
#define MEOW_ERROR_HPP_

#include <string>
#include "str_ref.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct error_t // fuck linux errno.h defining global typedef int error_t;
	{
	private:
		std::string what_;

	private:
		struct unspecified_bool_helper { int dummy; };
		typedef int unspecified_bool_helper::* unspecified_bool_type;

	public:
		error_t() {}
		explicit error_t(str_ref w) : what_(w.data(), w.size()) {}
		explicit error_t(std::string w) : what_(std::move(w)) {}

		error_t(error_t&& other) : what_(std::move(other.what_)) {}
		error_t(error_t const& other) : what_(other.what_) {}

		std::string const& what() const { return what_; }

		operator unspecified_bool_type() const
		{
			return what_.empty() ? 0 : &unspecified_bool_helper::dummy;
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#include <meow/format/metafunctions.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<>
	struct string_access<error_t>
	{
		static str_ref call(error_t const& e) { return e.what(); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_ERROR_HPP_
