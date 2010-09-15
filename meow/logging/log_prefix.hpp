////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOG_PREFIX_HPP_
#define MEOW_LOGGING__LOG_PREFIX_HPP_

#include <meow/format/format.hpp>
#include <meow/unix/time.hpp>
#include <meow/utility/bitmask.hpp>

#if defined(MEOW_LOGGING_PREFIX_TREE_ENABLED) && (MEOW_LOGGING_PREFIX_TREE_ENABLED != 0)
	#define MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
#endif

#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
	#include <boost/utility/enable_if.hpp>
	#include <boost/type_traits/is_base_and_derived.hpp>

	#include <meow/format/sink/std_string.hpp>

	#include <meow/tree/tree.hpp>
	#include <meow/tree/path_reconstruct.hpp>
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL

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

#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
		static int_t const log_path 	= 0x00000100;
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
	};
	typedef prefix_field::int_t prefix_fields_t;

	struct default_prefix_t
	{
		typedef default_prefix_t 	self_t;

#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
		typedef std::string 		log_path_t;
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL

	public:

		default_prefix_t()
		{
			prefix_fields_t f;
			bitmask_set(f, prefix_field::datetime);
			bitmask_set(f, prefix_field::log_level);
#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
			bitmask_clear(f, prefix_field::log_path);
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL

			this->set_prefix_fields(f);
		}

		prefix_fields_t prefix_fields() { return pf_; }
		void set_prefix_fields(prefix_fields_t const& pf) { pf_ = pf; }

	protected:

		typedef meow::tmp_buffer<64> buffer_t;

		template<class L>
		static str_ref prefix(L *log, log_level_t lvl, buffer_t const& buf = buffer_t())
		{
			return static_cast<self_t*>(log)->do_prefix(log, lvl, buf);
		}

	private:

#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
		template<class L>
		str_ref prefix_get_log_path(L *log, typename boost::enable_if<boost::is_base_and_derived<self_t, L>, L>::type *dummy=0)
		{
			if (log_path.empty())
				tree::reconstruct_path(log_path, tree::file_from_value(log));

			return str_ref(log_path);
		}
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL

		template<class L>
		str_ref do_prefix(L *log, log_level_t lvl, buffer_t const& buf)
		{
			using namespace meow::format;

			sink::char_buffer_sink_t sink(buf.begin(), buf.size());
			if (pf_ & prefix_field::datetime)
				fmt(sink, "[{0}]", format::as_log_ts(os_unix::gettimeofday_ex()));

#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
			if (pf_ & prefix_field::log_path)
				fmt(sink, "[{0}]", prefix_get_log_path(log));
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL

			if (pf_ & prefix_field::log_level)
				fmt(sink, "[{0}]", enum_as_str_ref(lvl));

			if (prefix_field::_null != pf_)
				fmt(sink, " ");

			return str_ref(buf.begin(), sink.size());
		}

	private:
		prefix_fields_t 	pf_;

#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
		log_path_t 			log_path;
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__LOG_PREFIX_HPP_

