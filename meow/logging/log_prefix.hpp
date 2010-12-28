////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOG_PREFIX_HPP_
#define MEOW_LOGGING__LOG_PREFIX_HPP_

#include <inttypes.h> // uint32_t

#include <meow/format/format.hpp>
#include <meow/unix/time.hpp>
#include <meow/utility/bitmask.hpp>

#include <meow/format/sink/char_buffer.hpp>

#include "format_timeval.hpp"
#include "log_level.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct prefix_field
	{
		typedef uint32_t int_t;

		static int_t const _null 		= 0x00000000;
		static int_t const datetime 	= 0x00000001;
		static int_t const log_level 	= 0x00000010;
		static int_t const log_name 	= 0x00000100;
	};
	typedef prefix_field::int_t prefix_fields_t;

////////////////////////////////////////////////////////////////////////////////////////////////

	struct default_prefix_t
	{
		default_prefix_t()
		{
			prefix_fields_t f;
			bitmask_set(f, prefix_field::datetime);
			bitmask_set(f, prefix_field::log_level);
			bitmask_clear(f, prefix_field::log_name);

			this->set_fields(f);
		}

		prefix_fields_t const& fields() { return pf_; }
		void set_fields(prefix_fields_t const& pf) { pf_ = pf; }

		std::string const& log_name() const { return log_name_; }
		void set_log_name(std::string const& n) { log_name_ = n; }

	public:

		typedef meow::tmp_buffer<64> buffer_t;

		str_ref prefix(log_level_t lvl, buffer_t const& buf = buffer_t())
		{
			using namespace meow::format;

			sink::char_buffer_sink_t sink(buf.begin(), buf.size());
			if (pf_ & prefix_field::datetime)
				write(sink, "[", format::as_log_ts(os_unix::gettimeofday_ex()), "]");

			if (pf_ & prefix_field::log_name)
				write(sink, "[", log_name_, "]");

			if (pf_ & prefix_field::log_level)
				write(sink, "[", enum_as_str_ref(lvl), "]");

			if (prefix_field::_null != pf_)
				write(sink, " ");

			return str_ref(buf.begin(), sink.size());
		}

	private:
		prefix_fields_t 	pf_;
		std::string 		log_name_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__LOG_PREFIX_HPP_

