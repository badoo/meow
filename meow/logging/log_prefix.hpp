////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOG_PREFIX_HPP_
#define MEOW_LOGGING__LOG_PREFIX_HPP_

#include <cstdint> // uint32_t

#include <meow/format/format.hpp>
#include <meow/format/sink/char_buffer.hpp>
#include <meow/unix/time.hpp>
#include <meow/utility/bitmask.hpp>

#include "format_timeval.hpp"
#include "log_level.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

	// struct prefix_field
	enum prefix_field : uint32_t
	{
		_null     = 0x00000000,
		_all      = 0xFFFFFFFF,

		datetime  = 0x00000001,
		log_level = 0x00000010,
		log_name  = 0x00000100,
	};
	typedef uint32_t prefix_fields_t;

////////////////////////////////////////////////////////////////////////////////////////////////

	struct default_prefix_t
	{
		default_prefix_t()
		{
			prefix_fields_t f = prefix_field::_null;
			BITMASK_SET(f, prefix_field::datetime);
			BITMASK_SET(f, prefix_field::log_level);
			BITMASK_CLEAR(f, prefix_field::log_name);

			this->set_fields(f);
		}

		prefix_fields_t const& fields() { return pf_; }
		void set_fields(prefix_fields_t const& pf) { pf_ = pf; }

		std::string const& log_name() const { return log_name_; }
		void set_log_name(std::string const& n) { log_name_ = n; }

	public:

		typedef meow::tmp_buffer<128> buffer_t;

		str_ref prefix(log_level_t lvl, buffer_t const& buf = buffer_t())
		{
			using namespace meow::format;

			char_buffer_sink_t sink(buf.begin(), buf.size());
			if (pf_ & prefix_field::datetime)
				write(sink, "[", format::as_log_ts(os_unix::clock_gettime_ex(CLOCK_REALTIME)), "]");

			if (pf_ & prefix_field::log_name)
				write(sink, "[", log_name_, "]");

			if (pf_ & prefix_field::log_level)
				write(sink, "[", log_level::enum_as_str_ref(lvl), "]");

			if (prefix_field::_null != pf_)
				write(sink, " ");

			return str_ref(buf.begin(), sink.size());
		}

	private:
		prefix_fields_t 	pf_;
		std::string 		log_name_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct empty_prefix_t
	{
		str_ref prefix(log_level_t lvl)
		{
			return {};
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__LOG_PREFIX_HPP_

