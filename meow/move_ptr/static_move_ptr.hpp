// (C) Copyright Jonathan Turkanis 2004.
// (C) Copyright Daniel Wallin 2004.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// Implementation of the move_ptr from the "Move Proposal"
// (http://std.dkuug.dk/jtc1/sc22/wg21/docs/papers/2002/n1377.htm)
// enhanced to support custom deleters and safe boolean conversions.
//
// The implementation is based on an implementation by Daniel Wallin, at
// "http://aspn.activestate.com/ASPN/Mail/Message/Attachments/boost/
// 400DC271.1060903@student.umu.se/move_ptr.hpp". The current was adapted
// by Jonathan Turkanis to incorporating ideas of Howard Hinnant and
// Rani Sharoni.

#ifndef MEOW_MOVE_PTR__STATIC_MOVE_PTR_HPP_
#define MEOW_MOVE_PTR__STATIC_MOVE_PTR_HPP_

#include <boost/config.hpp> // Member template friends, put size_t in std.
#include <algorithm>        // swap.
#include <cstddef>          // size_t
#include <boost/compressed_pair.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/is_array.hpp>

#include <meow/move_ptr/default_deleter.hpp>
#include <meow/move_ptr/is_convertible.hpp>
#include <meow/move_ptr/move.hpp>

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4521)        // Multiple copy constuctors.
#endif

namespace boost {

template<typename T, typename Deleter>
class static_move_ptr;

template<typename T>
struct is_const_static_move_ptr : mpl::false_ { };

template<typename T, typename Deleter>
struct is_const_static_move_ptr< const static_move_ptr<T, Deleter> >
    : mpl::true_
    { };

template< typename T,
          typename Deleter =
              move_ptrs::default_deleter<T> >
class static_move_ptr {
public:
    typedef typename remove_bounds<T>::type             element_type;
	typedef element_type 								value_type;
    typedef Deleter                                     deleter_type;
private:
    BOOST_STATIC_CONSTANT(bool, is_array = boost::is_array<T>::value);
//  this messes up apple gcc 4.0.1, maybe some others as well
//    BOOST_STATIC_ASSERT(!is_array || (is_same<T, element_type[]>::value));
    struct safe_bool_helper { int x; };
    typedef int safe_bool_helper::* safe_bool;
    typedef boost::compressed_pair<element_type*, Deleter> impl_type;
public:
    typedef typename impl_type::second_reference        deleter_reference;
    typedef typename impl_type::second_const_reference  deleter_const_reference;

        // Constructors

    static_move_ptr() : impl_(0) { }

    static_move_ptr(const static_move_ptr& p)
        : impl_(p.get(), p.get_deleter())
        {
            const_cast<static_move_ptr&>(p).release();
        }

    template<typename TT, typename DD>
    static_move_ptr( const static_move_ptr<TT, DD>& p,
                     typename
                     move_ptrs::enable_if_convertible<TT, T>::type* = 0 )
        : impl_( p.get(),
                 const_cast<typename add_reference<DD>::type>(p.get_deleter()) )
        {
            check(p);
            const_cast<static_move_ptr<TT, DD>&>(p).release();
        }

    template<typename TT, typename DD>
    static_move_ptr(move_ptrs::move_source< static_move_ptr<TT, DD> > src)
        : impl_(src.ptr().get(), src.ptr().get_deleter())
        {
            typedef move_ptrs::is_smart_ptr_convertible<TT, T> convertible;
            BOOST_STATIC_ASSERT(convertible::value);
            src.ptr().release();
        }

    template<typename TT>
    explicit static_move_ptr(TT* tt)
        : impl_(tt, Deleter())
        { }

    template<typename TT, typename DD>
    static_move_ptr(TT* tt, DD dd)
        : impl_(tt, dd)
        { }

        // Destructor

    ~static_move_ptr() { if (ptr()) get_deleter()(ptr()); }

        // Assignment

    static_move_ptr& operator=(static_move_ptr rhs)
        {
            rhs.swap(*this);
            return *this;
        }

    template<typename TT, typename DD>
    typename move_ptrs::enable_if_convertible<TT, T, static_move_ptr&>::type
    operator=(static_move_ptr<TT, DD> rhs)
        {
            static_move_ptr<T, Deleter> tmp(move(rhs));
            this->swap(tmp);
            return *this;
        }

        // Smart pointer interface

    element_type* get() const { return ptr(); }

    element_type& operator*() const
        {
            BOOST_STATIC_ASSERT(!is_array); return *ptr();
        }

    element_type* operator->() const
        {
            BOOST_STATIC_ASSERT(!is_array); return ptr();
        }

    element_type& operator[](std::size_t i) const
        {
            BOOST_STATIC_ASSERT(is_array); return ptr()[i];
        }

    element_type* release()
        {
            element_type* result = ptr();
            ptr() = 0;
            return result;
        }

    void reset()
        {
            if (ptr()) get_deleter()(ptr());
            ptr() = 0;
        }

    template<typename TT>
    void reset(TT* tt)
        {
            static_move_ptr(tt).swap(*this);
        }

    template<typename TT, typename DD>
    void reset(TT* tt, DD dd)
        {
            static_move_ptr(tt, dd).swap(*this);
        }

    operator safe_bool() const { return ptr() ? &safe_bool_helper::x : 0; }

    void swap(static_move_ptr& p) { impl_.swap(p.impl_); }

    deleter_reference get_deleter() { return impl_.second(); }

    deleter_const_reference get_deleter() const { return impl_.second(); }

private:
    template<typename TT, typename DD>
    void check(const static_move_ptr<TT, DD>& ptr)
	{
		typedef move_ptrs::is_smart_ptr_convertible<TT, T> convertible;
		BOOST_STATIC_ASSERT(convertible::value);
	}

#if 0
    template<typename Ptr> struct cant_move_from_const;

    template<typename TT, typename DD>
    struct cant_move_from_const< const static_move_ptr<TT, DD> > {
        typedef typename static_move_ptr<TT, DD>::error type;
    };
#endif

    template<typename Ptr, typename Enabler = void>
    struct cant_move_from_const;

    template<typename TT>
    struct cant_move_from_const<
              TT,
              typename enable_if<
                is_const_static_move_ptr<TT>
              >::type
           >
	{
		typedef typename TT::error type;
	};

	template<typename Ptr>
	static_move_ptr(Ptr&, typename cant_move_from_const<Ptr>::type = 0);

    static_move_ptr(static_move_ptr&);

    template<typename TT, typename DD>
    static_move_ptr( static_move_ptr<TT, DD>&,
                     typename
                     move_ptrs::enable_if_convertible<
                         TT, T, static_move_ptr&
                     >::type::type* = 0 );
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
    template<typename TT, typename DD>
    friend class static_move_ptr;
#else
    public:
#endif
    typename impl_type::first_reference
    ptr() { return impl_.first(); }

    typename impl_type::first_const_reference
    ptr() const { return impl_.first(); }

    impl_type impl_;
};

template<class T, class D>
T* get_pointer(static_move_ptr<T, D> const& p) { return p.get(); }

template<typename T>
static_move_ptr<T> move_raw(T *x)
{
	static_move_ptr<T> p(x);
	return move(p);
}

} // End namespace boost.

#if defined(BOOST_MSVC)
#pragma warning(pop) // #pragma warning(disable:4251)
#endif

#endif // #ifndef MEOW_MOVE_PTR__STATIC_MOVE_PTR_HPP_

