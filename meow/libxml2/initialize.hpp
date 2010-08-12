////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2006 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBXML2__INITIALIZE_HPP_
#define MEOW_LIBXML2__INITIALIZE_HPP_

#include <libxml/parser.h>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
// simple generic error handlers

	inline void silent_error_handler(void *ctx, char const *msg, ...) {}
	
////////////////////////////////////////////////////////////////////////////////////////////////
// initializer

	void parser_initialize(xmlGenericErrorFunc error_func = NULL)
	{
		xmlInitParser();
		LIBXML_TEST_VERSION;

		if (error_func) {
			static xmlGenericErrorFunc ef = error_func;
			initGenericErrorDefaultFunc(&ef);
		}

		struct parser_cleanup_t { ~parser_cleanup_t() { xmlCleanupParser(); } };
		static parser_cleanup_t parser_cleanup_;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBXML2__INITIALIZE_HPP_


