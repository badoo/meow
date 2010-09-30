////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOG_WRITE_HPP_
#define MEOW_LOGGING__LOG_WRITE_HPP_

#include <meow/format/format.hpp>
#include <meow/format/metafunctions.hpp>

#include "logger.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class LoggerT>
	struct logger_sink_t
	{
		typedef logging::log_level_t log_level_t;

		LoggerT 		*log;
		log_level_t 	level;
		line_mode_t 	lmode;

		logger_sink_t(LoggerT *l, log_level_t lvl, line_mode_t lm)
			: log(l)
			, level(lvl)
			, lmode(lm)
		{
		}
	};

	template<class LoggerT>
	struct sink_write<logger_sink_t<LoggerT> >
	{
		static void call(
				  logger_sink_t<LoggerT>& l
				, size_t total_len
				, typename LoggerT::str_t const* slices
				, size_t n_slices
				)
		{
			l.log->write(l.level, l.lmode, total_len, slices, n_slices);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}}} // namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#define LOG_DEFINE_LOG_WRITE_FUNCTION(z, n, d) 								\
	template<class LoggerT, class F FMT_TEMPLATE_PARAMS(n)> 				\
	void log_write( 														\
			  LoggerT& log 													\
			, log_level_t lvl 												\
			, line_mode_t lmode 											\
			, F const& fmt 													\
			  FMT_DEF_PARAMS(n)) 											\
	{ 																		\
		typedef format::sink::logger_sink_t<LoggerT> sink_t; 				\
		sink_t sink(&log, lvl, lmode); 										\
		meow::format::fmt(sink, fmt FMT_CALL_SITE_ARGS(n)); 				\
	} 																		\
/**/

	BOOST_PP_REPEAT(32, LOG_DEFINE_LOG_WRITE_FUNCTION, _);

////////////////////////////////////////////////////////////////////////////////////////////////

#define LOG_GENERIC_LEVEL_I(l, lvl, lmode, args...) 	\
	do { if ((l)->does_accept(lvl)) 					\
		log_write(*l, lvl, lmode, args); 				\
	} while(0) 											\
/**/

#define LOG_GENERIC_LEVEL(l, lvl, lmode, args...) 		\
	LOG_GENERIC_LEVEL_I(l, meow::logging::log_level::lvl, lmode, args) 	\
/**/

#define LOG_EMERG_EX(l, lmode, args...) 	LOG_GENERIC_LEVEL(l, emerg, lmode, args)
#define LOG_ALERT_EX(l, lmode, args...) 	LOG_GENERIC_LEVEL(l, alert, lmode, args)
#define LOG_CRIT_EX(l, lmode, args...) 		LOG_GENERIC_LEVEL(l, crit, lmode, args)
#define LOG_ERROR_EX(l, lmode, args...) 	LOG_GENERIC_LEVEL(l, error, lmode, args)
#define LOG_WARN_EX(l, lmode, args...) 		LOG_GENERIC_LEVEL(l, warn, lmode, args)
#define LOG_NOTICE_EX(l, lmode, args...) 	LOG_GENERIC_LEVEL(l, notice, lmode, args)
#define LOG_INFO_EX(l, lmode, args...) 		LOG_GENERIC_LEVEL(l, info, lmode, args)
#define LOG_DEBUG_EX(l, lmode, args...) 	LOG_GENERIC_LEVEL(l, debug, lmode, args)

#define LOG_EMERG(l, args...) 	LOG_EMERG_EX(l, meow::line_mode::single, args)
#define LOG_ALERT(l, args...) 	LOG_ALERT_EX(l, meow::line_mode::single, args)
#define LOG_CRIT(l, args...) 	LOG_CRIT_EX(l, meow::line_mode::single, args)
#define LOG_ERROR(l, args...) 	LOG_ERROR_EX(l, meow::line_mode::single, args)
#define LOG_WARN(l, args...) 	LOG_WARN_EX(l, meow::line_mode::single, args)
#define LOG_NOTICE(l, args...) 	LOG_NOTICE_EX(l, meow::line_mode::single, args)
#define LOG_INFO(l, args...) 	LOG_INFO_EX(l, meow::line_mode::single, args)
#define LOG_DEBUG(l, args...) 	LOG_DEBUG_EX(l, meow::line_mode::single, args)

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__LOG_WRITE_HPP_

