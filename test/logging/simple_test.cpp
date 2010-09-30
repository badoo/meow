////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////
//
// cd meow/test/format/
// g++ -O0 -g3 -o simple_test -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.41.0 simple_test.cpp
//

#include <boost/bind.hpp>

#include <meow/logging/logger.hpp>
#include <meow/logging/logger_impl.hpp>
#include <meow/logging/log_prefix.hpp>
#include <meow/logging/log_write.hpp>

#include <meow/format/sink/FILE.hpp>
#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////

using namespace meow;
using namespace meow::logging;

void log_generic_printf(FILE *to_file, size_t l, str_ref const* slices, size_t n_slices)
{
	format::sink::sink_write<FILE*>::call(to_file, l, slices, n_slices);
}

int main()
{
	typedef logger_t log_t;
	typedef logger_impl_t<default_prefix_t, log_t::char_t> limpl_t;
	typedef logger_proxy_t<log_t::char_t> lproxy_t;
	typedef logger_prefix_proxy_t<default_prefix_t, log_t::char_t> lprefix_proxy_t;

	limpl_t *l = new limpl_t();

	l->set_level(log_level::debug);
	l->set_writer(boost::bind(&log_generic_printf, stdout, _1, _2, _3));

	prefix_fields_t pf 	= prefix_field::datetime
						| prefix_field::log_level
						| prefix_field::log_name
						;
	l->prefix().set_fields(pf);
	l->prefix().set_log_name("impl");

	LOG_DEBUG(l, "test log, {0}", 10);

	{
		log_t *ll = new lproxy_t(l);
		ll->set_level(log_level::info);
		LOG_DEBUG(ll, "pure proxy: this log won't be printed");
		LOG_ALERT(ll, "pure proxy: this log is to be shown");
	}

	{
		lprefix_proxy_t *ll = new lprefix_proxy_t(l);
		ll->set_level(log_level::info);
		ll->prefix().set_fields(pf);
		ll->prefix().set_log_name("proxy");

		LOG_DEBUG(ll, "prefix: this log won't be printed");
		LOG_ALERT(ll, "prefix: this log is to be shown");
	}

	return 0;
}

