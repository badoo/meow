////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set ft=cpp ai noet ts=4 sw=4 fdm=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_MAPPING__COMMAND_LINE_HPP_
#define MEOW_MAPPING__COMMAND_LINE_HPP_

#ifndef _GNU_SOURCE
#	define _GNU_SOURCE	// required to get getopt_long from getopt.h
#endif
#include <getopt.h>		// for getopt_long()

#include <cstdarg> 		// va_start, etc.
#include <limits>
#include <string>
#include <vector>
#include <exception>

#include <boost/assert.hpp>
#include <boost/function.hpp>

#include <meow/str_ref.hpp>
#include <meow/tmp_buffer.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

/*
// NOTE: on all unixes i have around(linux + macosx) getopt defines
no_argument            0
required_argument      1
optional_argument      2
so we'll rely on this behaviour and define these constants even when they're not defined
*/
#ifndef no_argument
#	define no_argument 0
#endif

#ifndef required_argument
#	define required_argument 1
#endif

#ifndef optional_argument
#	define optional_argument 2
#endif

////////////////////////////////////////////////////////////////////////////////////////////////

	typedef enum {
		  arg_none = no_argument
		, arg_required = required_argument
		, arg_optional = optional_argument
	} argmode_t;

	struct optinfo_t
	{
		char opt_char;		// if an option has short variant, here it is
		char const *name;	// the long option name
		argmode_t argmode;
	};

	typedef std::string shopt_string_t;

	typedef struct ::option sys_option_t;
	typedef std::vector<sys_option_t> lopts_holder_t;

////////////////////////////////////////////////////////////////////////////////////////////////
namespace cmdline_detail {
////////////////////////////////////////////////////////////////////////////////////////////////

	inline optinfo_t make_mixed_option(char const *lname, char opt_char, argmode_t am)
	{
		optinfo_t opt = { opt_char: opt_char, name: lname, argmode: am };
		return opt;
	}

	inline optinfo_t make_long_option(char const *lname, argmode_t am)
	{
		return make_mixed_option(lname, char(0), am);
	}

	// make a short options string, sutable for handing to getopt/getopt_long
	//  don't care about memory management here
	//  maybe a version with explicit mm will be required, and maybe the one with temp buf

	template<class OptinfoT>
	inline shopt_string_t make_short_optstring(OptinfoT const *opts, size_t opts_size)
	{
		shopt_string_t shopts;

		shopts.push_back(':');

		for (OptinfoT const *i = opts, *i_end = opts + opts_size; i != i_end; ++i)
		{
			if (i->opt_char)
			{
				shopts.push_back(i->opt_char);

				switch (i->argmode)
				{
					case arg_optional:
						shopts.append("::");
						break;
					case arg_required:
						shopts.push_back(':');
						break;
					case arg_none:
						break;
				}
			}
		}

		return shopts;
	}

	// make a vector of 'long option' structs that can be passed to getopt_long()
	//  don't care about mm here
	//  call should be like this:
	//  lopts_holder_t h = make_long_optarray(...);
	//  getopt_long(argc, argv, short_opts, &*h.begin(), 0)
	//
	// @param: id_offset - start sys_option_t->val values from this number
	//						used to avoid clashing with other options
	template<class OptinfoT>
	inline lopts_holder_t make_long_optarray(OptinfoT const *opts, size_t opts_size, size_t id_offset)
	{
		BOOST_ASSERT(opts_size < id_offset); // checking that we're able to satify request at this offset

		lopts_holder_t lopts;

		for (OptinfoT const *i = opts, *i_end = opts + opts_size; i != i_end; ++i)
		{
			if (!i->name)
				continue;
			sys_option_t const lopt = { i->name, static_cast<int>(i->argmode), 0, id_offset + (i - opts) };
			lopts.push_back(lopt);
		}

		static sys_option_t const null_option = {};
		lopts.push_back(null_option);

		return lopts;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace cmdline_detail {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
// an object representing a mapping from command-line options to some used-defined data (ContextT)
//  the mapping is specified by a set of handling functions depending on ContextT

	template<class ContextT>
	struct cmdline_mapping_t
	{
		typedef cmdline_mapping_t	self_t;
		typedef ContextT			context_t;
		typedef boost::function<void(ContextT&, str_ref)> handler_fn_t;

		struct opt_trampoline_t : public optinfo_t
		{
			handler_fn_t handler_fn;

			opt_trampoline_t(optinfo_t const& opt_i, handler_fn_t const& h)
				: optinfo_t(opt_i)
				, handler_fn(h)
			{
			}
		};

		template<class HandlerF>
		self_t& option(optinfo_t const& opt_i, HandlerF const& handler)
		{
			opt_trampoline_t opt(opt_i, handler);
			opts_.push_back(opt);
			return *this;
		}

		template<class HandlerF>
		self_t& short_option(char opt_char, argmode_t am, HandlerF const& handler)
		{
			return this->option(cmdline_detail::make_mixed_option(NULL, opt_char, am), handler);
		}

		template<class HandlerF>
		self_t& mixed_option(char const *lname, char opt_char, argmode_t am, HandlerF const& handler)
		{
			return this->option(cmdline_detail::make_mixed_option(lname, opt_char, am), handler);
		}

		template<class HandlerF>
		self_t& long_option(char const *lname, argmode_t am, HandlerF const& handler)
		{
			return this->option(cmdline_detail::make_long_option(lname, am), handler);
		}

	private: // short options

		opt_trampoline_t const* get_short_option(int id) const
		{
			BOOST_ASSERT((0 <= id) && (id < (int)std::numeric_limits<char>::max()));
			for (opts_iterator_t i = opts_.begin(); i != opts_.end(); ++i)
			{
				opt_trampoline_t const *opt = &*i;
				if (opt->opt_char == (char)id)
					return opt;
			}
			return NULL;
		}

	private: // long options

		static size_t const lopt_offset = 10000; // number of short options must never reach this number

		opt_trampoline_t const* get_long_option(int id) const
		{
			BOOST_ASSERT(id >= 0);
			return ((lopt_offset <= size_t(id)) && (size_t(id) < lopt_offset + opts_.size()))
				? &opts_[id - lopt_offset]
				: NULL
				;
		}

	public: // external interface

		shopt_string_t short_optstring() const { return cmdline_detail::make_short_optstring(&*opts_.begin(), opts_.size()); }
		lopts_holder_t long_optarray() const { return cmdline_detail::make_long_optarray(&*opts_.begin(), opts_.size(), lopt_offset); }

		opt_trampoline_t const* get_option(int id) const
		{
			BOOST_ASSERT(id >= 0);
			return (size_t(id) >= lopt_offset) ? get_long_option(id) : get_short_option(id);
		}

	private:
		typedef std::vector<opt_trampoline_t>			opts_holder_t;
		typedef typename opts_holder_t::const_iterator	opts_iterator_t;
		opts_holder_t opts_;
	};	

////////////////////////////////////////////////////////////////////////////////////////////////
// exception classes specific to cmdline mapping
//  the base class provides a bit of convenience serives to it's children

	struct cmdmap_exception_base : virtual public std::exception
	{
	protected: // intended to be inherited from
		virtual ~cmdmap_exception_base() throw() {}

		mutable char buf[0x200];

		char const* format_tmp_str(char const *fmt, ...) const
		{
			va_list ap;
			va_start(ap, fmt);
			vsnprintf(buf, sizeof(buf), fmt, ap);
			va_end(ap);
			return buf;
		}

		typedef tmp_buffer<0x100> optname_buffer_t;
		char const* option_name_tmp_str(optinfo_t const& option_i, optname_buffer_t const& buf = optname_buffer_t()) const
		{
			int n = 0;
			if (option_i.opt_char)
				n = snprintf(buf.get(), buf.size(), "-%c, --%s", option_i.opt_char, option_i.name);
			else
				n = snprintf(buf.get(), buf.size(), "--%s", option_i.name);

			BOOST_ASSERT((n > 0) && (size_t(n) <= buf.size()) && "option long name is tooooo long");
			return buf.get();
		}
	};

	struct cmdmap_unknown_option : virtual public cmdmap_exception_base
	{
		char const *opt_name;
		cmdmap_unknown_option(char const *n) : opt_name(n) {}
		virtual char const* what() const throw() { return format_tmp_str("unknown option '%s'", opt_name); }
	};

	struct cmdmap_need_argument : virtual public cmdmap_exception_base
	{
		optinfo_t const option_info;
		cmdmap_need_argument(optinfo_t const& opt_i) : option_info(opt_i) {}
		virtual char const* what() const throw() { return format_tmp_str("option needs argument %s", option_name_tmp_str(option_info)); }
	};

	struct cmdmap_extra_parameter : virtual public cmdmap_exception_base
	{
		optinfo_t const option_info;
		cmdmap_extra_parameter(optinfo_t const& opt_i) : option_info(opt_i) {}
		virtual char const* what() const throw() { return format_tmp_str("extraneous arg for option %s", option_name_tmp_str(option_info)); }
	};

	struct cmdmap_handler_error : virtual public cmdmap_exception_base
	{
		cmdmap_handler_error(optinfo_t const& opt_i, char const *what)
		{
			format_tmp_str("error for option %s : %s", option_name_tmp_str(opt_i), what);
		}

		virtual char const* what() const throw() { return buf; }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
// the real command line processing and handling mapping

	template<class ContextT>
	void map_command_line(int argc, char **argv, cmdline_mapping_t<ContextT> const& mapping, ContextT *ctx)
	{
		shopt_string_t shopts = mapping.short_optstring();
		lopts_holder_t lopts = mapping.long_optarray();

		typedef typename cmdline_mapping_t<ContextT>::opt_trampoline_t opt_trampoline_t;

		// getopt'ing at last
		while (true)
		{
			int c = ::getopt_long(argc, argv, shopts.c_str(), &*lopts.begin(), 0);
			if (-1 == c) // all done
			{
				break;
			}
			else if ('?' == c) // ambiguous option or extra arg
			{
				if (0 == ::optopt) // unknown option
					throw cmdmap_unknown_option(argv[::optind - 1]);

				opt_trampoline_t const* trampo = mapping.get_option(::optopt);
				BOOST_ASSERT((NULL != trampo) && "option can't be not found after we checked if it's valid");
				throw cmdmap_extra_parameter(*trampo);
			}
			else
			{
				if (':' == c) // no argument for option with required arg
					c = ::optopt;

				opt_trampoline_t const* trampo = mapping.get_option(c);
				BOOST_ASSERT(trampo && "nonexistent opts must've been reported already");
				try
				{
					trampo->handler_fn(*ctx, ::optarg ? str_ref(::optarg) : str_ref());
				}
				catch (std::exception const& e)
				{
					throw cmdmap_handler_error(*trampo, e.what());
				}
			}

		} // while
	}

	template<class ContextT>
	ContextT map_command_line(int argc, char **argv, cmdline_mapping_t<ContextT> const& mapping)
	{
		ContextT ctx;
		map_command_line(argc, argv, mapping, &ctx);
		return ctx;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace mapping {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_MAPPING__COMMAND_LINE_HPP_

