////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LOGGING__LOGGER_IMPL_HPP_
#define MEOW_LOGGING__LOGGER_IMPL_HPP_

#include <functional> // function

#include <meow/format/format.hpp>
#include <meow/format/metafunctions.hpp>

#include "log_level.hpp"
#include "logger.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class PrefixT, class CharT = char>
	struct logger_impl_t : public logger_base_t<CharT>
	{
		typedef logger_base_t<CharT> 	base_t;
		typedef PrefixT 				prefix_t;
		typedef typename base_t::str_t 	str_t;
		typedef std::function<void(size_t, str_t const*, size_t)> writer_fn_t;

	public:

		prefix_t& prefix() { return prefix_; }

		writer_fn_t const& writer() const { return writer_; }
		void set_writer(writer_fn_t const& w) { writer_ = w; }

		void do_write(size_t total_len, str_t const *slices, size_t n_slices)
		{
			BOOST_ASSERT(writer_);
			writer_(total_len, slices, n_slices);
		}

	public:

		virtual log_level_t level() const { return level_; }
		virtual log_level_t set_level(log_level_t l) { return level_ = l; }

		virtual bool does_accept(log_level_t l) const { return l <= level_; }

		virtual void write(
						  log_level_t 	lvl
						, line_mode_t 	lmode
						, size_t 		total_len
						, str_t const 	*slices
						, size_t 		n_slices
						)
		{
			if (line_mode::prefix & lmode)
				format::write(*this, prefix_.prefix(lvl));

			this->do_write(total_len, slices, n_slices);

			if (line_mode::suffix & lmode)
				format::write(*this, ref_lit("\n"));
		}

	public:

		explicit logger_impl_t(log_level_t lvl = log_level::off)
			: level_(lvl)
			, writer_()
		{
		}

		explicit logger_impl_t(writer_fn_t const& w, log_level_t lvl = log_level::off)
			: base_t(lvl)
			, writer_(w)
		{
		}

	private:
		log_level_t level_;
		prefix_t 	prefix_;
		writer_fn_t writer_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class CharT = char>
	struct logger_proxy_t : public logger_base_t<CharT>
	{
		typedef logger_base_t<CharT> 	base_t;
		typedef base_t 					target_t;
		typedef typename base_t::str_t 	str_t;

	public:

		virtual void write(
						  log_level_t 	lvl
						, line_mode_t 	lmode
						, size_t 		total_len
						, str_t const 	*slices
						, size_t 		n_slices
						)
		{
			BOOST_ASSERT(target_);

			if (target_->does_accept(lvl))
				target_->write(lvl, lmode, total_len, slices, n_slices);
		}

	public:

		explicit logger_proxy_t(target_t *target, log_level_t lvl = log_level::off)
			: base_t(lvl)
			, target_(target)
		{
		}

	private:
		target_t *target_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class PrefixT, class CharT = char>
	struct logger_prefix_proxy_t : public logger_base_t<CharT>
	{
		typedef logger_base_t<CharT> 	base_t;
		typedef PrefixT 				prefix_t;
		typedef base_t 					target_t;
		typedef typename base_t::str_t 	str_t;

	public:

		prefix_t& prefix() { return prefix_; }

		virtual void write(
						  log_level_t 	lvl
						, line_mode_t 	lmode
						, size_t 		total_len
						, str_t const 	*slices
						, size_t 		n_slices
						)
		{
			BOOST_ASSERT(target_);

			if (!target_->does_accept(lvl))
				return;

			if (line_mode::prefix & lmode)
			{
				lmode &= ~line_mode::prefix;
				log_write(*target_, lvl, line_mode::middle, prefix_.prefix(lvl));
			}

			target_->write(lvl, lmode, total_len, slices, n_slices);
		}

	public:

		explicit logger_prefix_proxy_t(target_t *target, log_level_t lvl = log_level::off)
			: base_t(lvl)
			, target_(target)
		{
		}

	private:
		PrefixT 	prefix_;
		target_t 	*target_;
	};

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace logging {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format { namespace sink {
////////////////////////////////////////////////////////////////////////////////////////////////

	// a custom meow.format sink that logger can be used as

	template<class Traits, class CharT>
	struct sink_write<meow::logging::logger_impl_t<Traits, CharT> >
	{
		static void call(
				  meow::logging::logger_impl_t<Traits, CharT>& l
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

