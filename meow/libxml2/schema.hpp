////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2006 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBXML2__SCHEMA_HPP_
#define MEOW_LIBXML2__SCHEMA_HPP_

#include <libxml/xmlschemas.h>

#include "core.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

	MEOW_DEFINE_LIBXML2_OBJECT(Schema, ,			schema_ptr_t);
	MEOW_DEFINE_LIBXML2_OBJECT(Schema, ParserCtxt,	schema_parser_ctx_ptr_t);
	MEOW_DEFINE_LIBXML2_OBJECT(Schema, ValidCtxt,	schema_valid_ctx_ptr_t);

	template<class BufferT>
	bool schema_validate_document(BufferT const& xml, BufferT const& xsd)
	{
		// parse xsd
		schema_parser_ctx_ptr_t parser_ctx = acquire_move_ptr(xmlSchemaNewMemParserCtxt(xsd.begin(), xsd.size()));
		schema_ptr_t schema = acquire_move_ptr(xmlSchemaParse(parser_ctx.get()));
		schema_valid_ctx_ptr_t valid_ctx = acquire_move_ptr(xmlSchemaNewValidCtxt(schema.get()));

		// parse xml
		doc_ptr_t doc = acquire_move_ptr(xmlParseMemory(xml.begin(), xml.size()));

		// validate
		int ret = xmlSchemaValidateDoc(valid_ctx.get(), doc.get());
		if (0 == ret) { return true; }
		else if (ret > 0) { return false; }
		else { throw generic_error_t(); }
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBXML2__SCHEMA_HPP_

