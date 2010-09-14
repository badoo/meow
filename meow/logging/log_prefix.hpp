////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOG_PREFIX_HPP_
#define MEOW_LOGGING__LOG_PREFIX_HPP_

#include <meow/bitfield_union.hpp>
#include <meow/format/format.hpp>

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

#include <meow/unix/time.hpp>

#include "log_level.hpp"
#include "format_timeval.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct default_prefix_t
	{
		typedef default_prefix_t 	self_t;

#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
		typedef std::string 		log_path_t;
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL

		struct enabled_fields_data_t
		{
			bool datetime 	: 1;
			bool log_level 	: 1;
#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
			bool log_path 	: 1;
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL

		};
		typedef meow::bitfield_union<enabled_fields_data_t, uint32_t> enabled_fields_t;

	public:

		default_prefix_t()
		{
			enabled_fields_data_t const f = {
				  datetime: true
				, log_level: true
#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
				, log_path: false
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
			};
			this->set_prefix_fields(f);
		}

		enabled_fields_t const& enabled_fields() { return ef_; }
		void set_prefix_fields(enabled_fields_t const& ef) { ef_ = ef; }
		void set_prefix_fields(enabled_fields_data_t const& efd) { ef_ = efd; }

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
			if (ef_->datetime)
				fmt(sink, "[{0}]", format::as_log_ts(os_unix::gettimeofday_ex()));

#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
			if (ef_->log_path)
				fmt(sink, "[{0}]", prefix_get_log_path(log));
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL

			if (ef_->log_level)
				fmt(sink, "[{0}]", enum_as_str_ref(lvl));
			if (0 != ef_)
				fmt(sink, " ");
			return str_ref(buf.begin(), sink.size());
		}

	private:
		enabled_fields_t 	ef_;

#ifdef MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
		log_path_t 			log_path;
#endif // MEOW_LOGGING_PREFIX_TREE_ENABLED_INTERNAL
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__LOG_PREFIX_HPP_

