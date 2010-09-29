////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOGGER_IMPL_HPP_
#define MEOW_LOGGING__LOGGER_IMPL_HPP_

#include <boost/function.hpp>

#include <meow/format/format.hpp>
#include <meow/format/metafunctions.hpp>

#include <meow/utility/line_mode.hpp>

#include "log_level.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#define LOG_GENERIC_LEVEL_I(l, lvl, lmode, args...) 	\
	do { if ((l)->does_accept(lvl)) 					\
		log_write((*(l)), lvl, lmode, args); 			\
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

	template<class PrefixT, class CharT = char>
	struct logger_impl_t : public PrefixT
	{
		typedef logger_impl_t 				self_t;
		typedef PrefixT 					prefix_t;
		typedef string_ref<CharT const> 	str_t;
		typedef boost::function<void(size_t, str_t const*, size_t)> writer_fn_t;

	private:
		log_level_t 	level_;
		writer_fn_t 	writer_;

	public:

		logger_impl_t()
			: level_(log_level::off)
		{
		}

		explicit logger_impl_t(writer_fn_t const& w)
			: level_(log_level::off)
			, writer_(w)
		{
		}

		log_level_t level() const { return level_; }
		log_level_t set_level(log_level_t l) { return level_ = l; }
		bool does_accept(log_level_t const l) const { return l <= level_; }

		writer_fn_t const& writer() const { return writer_; }
		void set_writer(writer_fn_t const& w) { writer_ = w; }

		void do_write(size_t total_len, str_t const *slices, size_t n_slices)
		{
			BOOST_ASSERT(writer_);
			writer_(total_len, slices, n_slices);
		}

	};

#define LOG_DEFINE_WRITE_FREE_FUNCTION(z, n, d) 							\
	template<class PrefixT, class F FMT_TEMPLATE_PARAMS(n)> 				\
	void log_write( 														\
			  logger_impl_t<PrefixT>& log 									\
			, log_level_t lvl 												\
			, line_mode_t lmode 											\
			, F const& fmt 													\
			  FMT_DEF_PARAMS(n)) 											\
	{ 																		\
		if (!log.writer()) 													\
			return; 														\
		if (line_mode::prefix & lmode) 										\
			meow::format::write(log, PrefixT::prefix(&log, lvl)); 			\
		meow::format::fmt(log, fmt FMT_CALL_SITE_ARGS(n)); 					\
		if (line_mode::suffix & lmode) 										\
			meow::format::write(log, "\n"); 								\
	} 																		\
/**/

	BOOST_PP_REPEAT(32, LOG_DEFINE_WRITE_FREE_FUNCTION, _);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

	// a custom meow.format sink that logger can be used as

	template<class Traits>
	struct sink_write<meow::logging::logger_impl_t<Traits> >
	{
		template<class CharT>
		static void call(
				  meow::logging::logger_impl_t<Traits>& l
				, size_t total_len
				, meow::string_ref<CharT const> const* slices
				, size_t n_slices
				)
		{
			l.do_write(total_len, slices, n_slices);
		}
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}}} // namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__LOGGER_IMPL_HPP_

