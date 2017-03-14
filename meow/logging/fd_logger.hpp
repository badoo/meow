////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__FD_LOGGER_HPP_
#define MEOW_LOGGING__FD_LOGGER_HPP_

#include <meow/format/sink/fd.hpp>
#include <meow/logging/log_prefix.hpp>
#include <meow/logging/logger_impl.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class PrefixT = default_prefix_t, class CharT = char>
	struct fd_logger_t : public logger_impl_t<PrefixT, CharT>
	{
		using base_t = logger_impl_t<PrefixT, CharT>;

		explicit fd_logger_t(int fd, log_level_t lvl = log_level::off)
			: base_t(lvl)
			, sink_(fd)
		{
			this->set_writer([this](size_t total_len, str_ref const *slices, size_t n_slices) {
				meow::format::write(sink_, total_len, slices, n_slices);
			});
		}

	private:
		meow::format::fd_sink_t sink_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LOGGING__FD_LOGGER_HPP_
