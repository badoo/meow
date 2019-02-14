////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__MOVABLE_HANDLE_HPP_
#define MEOW__MOVABLE_HANDLE_HPP_

#include <utility> // for std::swap, since C++11

#include <boost/call_traits.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class Traits>
	struct movable_handle
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

	public:

		movable_handle()
			: handle_(traits_type::null())
		{
		}

		explicit movable_handle(param_type h)
			: handle_(h)
		{
		}

		movable_handle(movable_handle&& other)
			: handle_(other.release())
		{
		}

		movable_handle(movable_handle&) = delete;
		movable_handle(movable_handle const&) = delete;

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

	template<class T>
	T const& get_handle(T const& h) { return h; }

	template<class T>
	T& get_handle(T& h) { return h; }

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace meow {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__MOVABLE_HANDLE_HPP_

