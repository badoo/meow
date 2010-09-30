////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOGGER_HPP_
#define MEOW_LOGGING__LOGGER_HPP_

#include <boost/noncopyable.hpp>

#include <meow/str_ref.hpp>
#include <meow/utility/line_mode.hpp>
#include <meow/move_ptr/static_move_ptr.hpp>

#include "log_level.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT = char>
	struct logger_base_t : private boost::noncopyable
	{
		typedef CharT 						char_t;
		typedef string_ref<char_t const> 	str_t;

	public: // level is built into every logger regardless

		log_level_t level() const { return level_; }
		log_level_t set_level(log_level_t l) { return level_ = l; }

		bool does_accept(log_level_t l) const { return l <= level_; }

	public:

		// the virtual part, actually writing the thing
		//  log level is here only for informational purposes
		//  so logger is not required to check if it accepts it at all
		virtual void write(
						  log_level_t 	lvl
						, line_mode_t 	lmode
						, size_t 		total_len
						, str_t const 	*slices
						, size_t 		n_slices
						) = 0;

	public:

		logger_base_t(log_level_t lvl)
			: level_(lvl)
		{
		}

		virtual ~logger_base_t() {}

	private:
		log_level_t level_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef logger_base_t<char> 	logger_t;
	typedef logger_base_t<wchar_t>	w_logger_t;

	typedef boost::static_move_ptr<logger_t> 	logger_move_ptr;
	typedef boost::static_move_ptr<w_logger_t> 	w_logger_move_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__LOGGER_HPP_

