////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__MOVABLE_HANDLE_HPP_
#define MEOW__MOVABLE_HANDLE_HPP_

#include <algorithm> // for std::swap

#include <boost/call_traits.hpp>

#include <meow/movable.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits>
	struct movable_handle
		: public movable::move_enabled<movable_handle<Traits> >
	{
		typedef movable_handle		self_type;
		typedef Traits				traits_type;
		typedef typename traits_type::handle_type handle_type;

		typedef handle_type*		pointer;
		typedef handle_type const*	const_pointer;
		typedef typename boost::call_traits<handle_type>::reference			reference;
		typedef typename boost::call_traits<handle_type>::const_reference	const_reference;
		typedef typename boost::call_traits<handle_type>::param_type		param_type;

	private:

		struct unspecified_bool { int dummy; };
		typedef int unspecified_bool::* unspecified_bool_type;

		handle_type handle_;

	private:

		movable_handle(movable_handle&);

		template<class T> struct cant_move_from_const;

		template<class T>
		struct cant_move_from_const<const movable_handle<T> > { 
			typedef typename self_type::error type; 
		};

		// the const lvalue move catcher, disallow that
		template<class U>
		movable_handle(U&, typename cant_move_from_const<U>::type * = 0) {}

	public:

		movable_handle()
			: handle_(traits_type::null())
		{
		}

		explicit movable_handle(param_type h)
			: handle_(h)
		{
		}

		// move construction
		movable_handle(movable::temporary<self_type> other)
			: handle_(other.get().release())
		{
		}

		// copy construction from temporary, see the lvalue const catcher above
		movable_handle(movable_handle const& other) 
			: handle_(const_cast<self_type&>(other).release())
		{
		}

		~movable_handle()
		{
			if (!traits_type::is_null(handle_))
				traits_type::close(handle_);
		}

		self_type& operator=(self_type other)
		{
			if (this != &other)
				this->reset(other.release());
			return *this;
		}

		void swap(self_type& other)
		{
			using std::swap;
			swap(handle_, other.handle_);
		}

		void reset() { self_type().swap(*this); }
		void reset(param_type h) { self_type(h).swap(*this); }

	handle_type release()
	{
		handle_type t(handle_);
		handle_ = traits_type::null();
		return t;
	}

	const_reference get() const { return handle_; }
	reference ref() { return handle_; }

	pointer ptr() { return &handle_; }
	const_pointer ptr() const { return &handle_; }

	reference operator->() { return ref(); }
	const_reference operator->() const { return ref(); }

	operator unspecified_bool_type() const
	{
		return traits_type::is_null(handle_)
			? 0
			: &unspecified_bool::dummy
			;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	inline void swap(movable_handle<T>& l, movable_handle<T>& r)
	{
		l.swap(r);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits>
	inline
	typename movable_handle<Traits>::const_reference
	get_handle(movable_handle<Traits> const& h) { return h.get(); }

	template<class Traits>
	inline
	typename movable_handle<Traits>::reference
	get_handle(movable_handle<Traits>& h) { return h.ref(); }

	template<class Traits>
	inline
	typename movable_handle<Traits>::pointer
	get_pointer(movable_handle<Traits>& h) { return h.ptr(); }

	template<class Traits>
	inline
	typename movable_handle<Traits>::const_pointer
	get_pointer(movable_handle<Traits> const& h) { return h.ptr(); }

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__MOVABLE_HANDLE_HPP_

