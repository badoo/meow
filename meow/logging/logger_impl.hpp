////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOGGER_IMPL_HPP_
#define MEOW_LOGGING__LOGGER_IMPL_HPP_

#include <boost/function.hpp>

#include <meow/format/format.hpp>
#include <meow/format/metafunctions.hpp>

#include "log_level.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#define LOG_GENERIC_LEVEL_I(l, lvl, args...) 	\
	do { if ((l)->does_accept(lvl)) 			\
		(l)->write(lvl, args); 					\
	} while(0); 								\
/**/

#define LOG_GENERIC_LEVEL(l, lvl, args...) 				\
	LOG_GENERIC_LEVEL_I(l, meow::logging::lvl, args) 	\
/**/

#define LOG_EMERG(l, args...) 	LOG_GENERIC_LEVEL(l, log_level::emerg, args)
#define LOG_ALERT(l, args...) 	LOG_GENERIC_LEVEL(l, log_level::alert, args)
#define LOG_CRIT(l, args...) 	LOG_GENERIC_LEVEL(l, log_level::crit, args)
#define LOG_ERROR(l, args...) 	LOG_GENERIC_LEVEL(l, log_level::error, args)
#define LOG_WARN(l, args...) 	LOG_GENERIC_LEVEL(l, log_level::warn, args)
#define LOG_NOTICE(l, args...) 	LOG_GENERIC_LEVEL(l, log_level::notice, args)
#define LOG_INFO(l, args...) 	LOG_GENERIC_LEVEL(l, log_level::info, args)
#define LOG_DEBUG(l, args...) 	LOG_GENERIC_LEVEL(l, log_level::debug, args)

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class PrefixT, class CharT = char>
	struct logger_impl_t : public PrefixT
	{
		typedef logger_impl_t 	self_t;
		typedef PrefixT 		prefix_t;
		typedef boost::function<void(size_t, string_ref<CharT const> const*, size_t)> writer_fn_t;

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

		void set_writer(writer_fn_t const& w) { writer_ = w; }

		void do_write(size_t total_len, str_ref const *slices, size_t n_slices)
		{
			BOOST_ASSERT(writer_);
			writer_(total_len, slices, n_slices);
		}

#define LOG_DEFINE_WRITE_FUNCTION(z, n, d) 									\
		template<class F FMT_TEMPLATE_PARAMS(n)> 								\
		void write(log_level_t lvl, F const& fmt FMT_DEF_PARAMS(n)) 			\
		{ 																		\
			if (!writer_) 														\
				return; 														\
			meow::format::fmt(*this, "{0}", prefix_t::prefix(this, lvl)); 		\
			meow::format::fmt(*this, fmt FMT_CALL_SITE_ARGS(n)); 				\
			meow::format::fmt(*this, "\n"); 									\
		} 																		\
	/**/

		BOOST_PP_REPEAT(32, LOG_DEFINE_WRITE_FUNCTION, _);
	};

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

