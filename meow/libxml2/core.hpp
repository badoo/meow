////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2006 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_LIBXML2__CORE_HPP_
#define MEOW_LIBXML2__CORE_HPP_

#include <cstdio>				// for snprintf

#include <libxml/tree.h>		// for xmlDoc
#include <libxml/xmlerror.h>	// for xmlError
#include <libxml/xmlstring.h>	// for xmlChar

#include <exception>

#include <meow/move_ptr/static_move_ptr.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

	struct generic_error_t : virtual public std::exception
	{

		generic_error_t(xmlError *err = xmlGetLastError())
			: xmlerr_(err)
		{
			int n = snprintf(
					  buffer_, buffer_size
					, "%s:%d : (%d) %s"
					, err->file ? err->file : "[unknown]", err->line
					, err->code, err->message
				);
			buffer_[n] = 0x0;
		}

		virtual ~generic_error_t() throw() {}

		virtual char const* what() const throw() { return buffer_; }
		xmlError* xml_error() const throw() { return xmlerr_; }

	private:
		xmlError *xmlerr_;
		enum { buffer_size = 0x200 };
		char buffer_[buffer_size];
	};

////////////////////////////////////////////////////////////////////////////////////////////////
// object names
	#define MEOW_LIBXML2_COMPOSE_OBJNAME(ns, obj_name) xml ## ns ## obj_name
	#define MEOW_LIBXML2_COMPOSE_DELETER_NAME(ns, obj_name) xml ## ns ## Free ## obj_name
	
////////////////////////////////////////////////////////////////////////////////////////////////
// RAII wrappers for libxml objects

	struct deleter_t
	{
		// NOTE: calling a free function here
		//  this function should be overloaded for every libxml2 object
		//  we want to be able to delete
		// qualified call used to prevent unintentded ADL.
		template<class T>
		void operator()(T *obj) const { /*libxml2::*/delete_object(obj); }
	};

	#define MEOW_DEFINE_LIBXML2_OBJECT_DELETER(ns, obj_name)						\
		inline void delete_object(MEOW_LIBXML2_COMPOSE_OBJNAME(ns, obj_name) *p)	\
		{  MEOW_LIBXML2_COMPOSE_DELETER_NAME(ns, obj_name) (p); }

	#define MEOW_DEFINE_LIBXML2_PRIMITIVE_OBJECT_DELETER(ns, obj_name)			\
		inline void delete_object(MEOW_LIBXML2_COMPOSE_OBJNAME(ns, obj_name) *p)	\
		{ xmlFree(p); }

////////////////////////////////////////////////////////////////////////////////////////////////
// libxml object smart pointers

	template<class T>
	struct as_move_ptr { typedef typename boost::static_move_ptr<T, deleter_t> type; };

	template<class T>
	inline typename as_move_ptr<T>::type acquire_move_ptr(T *p)
	{
		if (NULL == p)
			throw generic_error_t();

		return typename as_move_ptr<T>::type(p);
	}

	// object wrappers
	#define MEOW_DEFINE_LIBXML2_OBJECT_MOVE_PTR(ns, obj_name, type_name) \
		typedef as_move_ptr< MEOW_LIBXML2_COMPOSE_OBJNAME(ns, obj_name) >::type type_name;

////////////////////////////////////////////////////////////////////////////////////////////////
// main object definition macro

	#define MEOW_DEFINE_LIBXML2_OBJECT(ns, obj_name, type_name)			\
		MEOW_DEFINE_LIBXML2_OBJECT_DELETER(ns, obj_name);				\
		MEOW_DEFINE_LIBXML2_OBJECT_MOVE_PTR(ns, obj_name, type_name);

	#define MEOW_DEFINE_LIBXML2_PRIMITIVE_OBJECT(ns, obj_name, type_name)	\
		MEOW_DEFINE_LIBXML2_PRIMITIVE_OBJECT_DELETER(ns, obj_name);			\
		MEOW_DEFINE_LIBXML2_OBJECT_MOVE_PTR(ns, obj_name, type_name);

	// objdefs normal
	MEOW_DEFINE_LIBXML2_OBJECT(, Doc, doc_ptr_t);

	// objdefs primitive
	MEOW_DEFINE_LIBXML2_PRIMITIVE_OBJECT(, Char, char_ptr_t);

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace libxml2 {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_LIBXML2__CORE_HPP_

