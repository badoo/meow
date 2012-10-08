////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBEV_SSL__BASE_TYPES_HPP_
#define MEOW_LIBEV_SSL__BASE_TYPES_HPP_

#include <meow/smart_enum.hpp>
#include <meow/utility/line_mode.hpp>
#include <meow/format/format_to_str_copy.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct ssl_log_writer_traits__default
	{
		template<class ContextT> static bool is_allowed(ContextT *ctx) { return false; }
		template<class ContextT> static void write(ContextT *ctx, meow::line_mode_t, str_ref const&) {}
	};

	#define SSL_LOG_WRITE(ctx, lmode, fmt, ...) 				\
		do { if (ssl_log_writer::is_allowed(ctx))				\
			ssl_log_writer::write(ctx, lmode, meow::format::fmt_str_copy(fmt, ##__VA_ARGS__));	\
		} while(0)												\
	/**/

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libev {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBEV_SSL__BASE_TYPES_HPP_

