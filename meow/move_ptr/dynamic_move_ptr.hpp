// (C) Copyright Jonathan Turkanis.
// (C) Copyright Daniel Wallin.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// Implementation of the move_ptr from the "Move Proposal" 
// (http://std.dkuug.dk/jtc1/sc22/wg21/docs/papers/2002/n1377.htm) 
// enhanced to support custom deleters and safe boolean conversions.
//
// The implementation is based on an implementation by Daniel Wallin, at
// "http://aspn.activestate.com/ASPN/Mail/Message/Attachments/boost/
// 400DC271.1060903@student.umu.se/move_ptr.hpp". The current was adapted 
// by Jonathan Turkanis to incorporating ideas of Howard Hinnant,
// Rani Sharoni and Bronek Kozecki.

#ifndef MEOW_MOVE_PTR__DYNAMIC_MOVE_PTR_HPP_
#define MEOW_MOVE_PTR__DYNAMIC_MOVE_PTR_HPP_

#include <boost/config.hpp> // Member template friends, put size_t in std.
#include <algorithm>        // swap.
#include <cstddef>          // size_t
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_array.hpp>
#include <boost/type_traits/remove_bounds.hpp>

#include <meow/move_ptr/bound_deleter.hpp>
#include <meow/move_ptr/is_convertible.hpp>
#include <meow/move_ptr/move.hpp>

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4521)        // Multiple copy constuctors.
#endif

namespace boost { 

template<typename T>
class dynamic_move_ptr {
public:
    typedef typename remove_bounds<T>::type             element_type;
private:
    BOOST_STATIC_CONSTANT(bool, is_array = boost::is_array<T>::value);
    struct safe_bool_helper { int x; };
    typedef int safe_bool_helper::* safe_bool;
public:

        // Constructors

    dynamic_move_ptr() : t_(0) { }

    dynamic_move_ptr(const dynamic_move_ptr& p)
        : deleter_(p.deleter_), t_(p.t_)
        { 
            const_cast<dynamic_move_ptr&>(p).release();
        }

    template<typename TT>
    dynamic_move_ptr( const dynamic_move_ptr<TT>& p,
                      typename 
                      move_ptrs::enable_if_convertible<TT, T>::type* = 0 )
        : deleter_(p.deleter_), t_(p.t_)
        { 
            check(p);
            const_cast<dynamic_move_ptr<TT>&>(p).release();
        }

    template<typename TT>
    dynamic_move_ptr(move_ptrs::move_source< dynamic_move_ptr<TT> > src)
        : deleter_(src.ptr().deleter_), t_(src.ptr().t_)
        {
            typedef move_ptrs::is_smart_ptr_convertible<TT, T> convertible;
            BOOST_STATIC_ASSERT(convertible::value);
            src.ptr().release();
        }

    template<typename TT>
    explicit dynamic_move_ptr(TT* tt) 
        : deleter_(tt), t_(tt)
        { }

    template<typename TT, typename DD>
    dynamic_move_ptr(TT* tt, DD dd) 
        : deleter_(tt, dd), t_(tt)
        { }

        // Destructor

    ~dynamic_move_ptr() { if (t_) deleter_.delete_(); }

        // Assignment

    dynamic_move_ptr& operator=(dynamic_move_ptr rhs)
        {
            rhs.swap(*this);
            return *this;
        }

    template<typename TT>    
    typename move_ptrs::enable_if_convertible<TT, T, dynamic_move_ptr&>::type
    operator=(dynamic_move_ptr<TT> rhs)
        {
            dynamic_move_ptr<T> tmp(move(rhs));
            this->swap(tmp);
            return *this;
        }

        // Smart pointer interface

    element_type* get() const { return t_; }

    element_type& operator*() const 
        { 
            BOOST_STATIC_ASSERT(!is_array); return *t_; 
        }

    element_type* operator->() const 
        { 
            BOOST_STATIC_ASSERT(!is_array); return t_; 
        }    

    element_type& operator[](std::size_t i) const 
        { 
            BOOST_STATIC_ASSERT(is_array); return t_[i]; 
        }

    element_type* release()
        {
            element_type* result = t_;
            t_ = 0;
            return result;
        }

    void reset()
        {
            if (t_) deleter_.delete_();
            t_ = 0;
        }

    template<typename TT>
    void reset(TT* tt) 
        {
            dynamic_move_ptr<T> tmp(tt);
            reset();
            this->swap(tmp);
        }

    template<typename TT, typename DD>
    void reset(TT* tt, DD dd) 
        {
            dynamic_move_ptr<T> tmp(tt, dd);
            reset();
            this->swap(tmp);
        }

    operator safe_bool() const { return t_ ? &safe_bool_helper::x : 0; }

    void swap(dynamic_move_ptr& p) 
        { 
            std::swap(deleter_, p.deleter_);
            std::swap(t_, p.t_);
        }
private:
    template<typename TT>
    void check(const dynamic_move_ptr<TT>& ptr)
        {
            typedef move_ptrs::is_smart_ptr_convertible<TT, T> convertible;
            BOOST_STATIC_ASSERT(convertible::value);
        }   

    template<typename Ptr> struct cant_move_from_const;

    template<typename TT> 
    struct cant_move_from_const< const dynamic_move_ptr<TT> >
    { 
        typedef typename dynamic_move_ptr<TT>::error type; 
    };

    template<typename Ptr> 
    dynamic_move_ptr(Ptr&, typename cant_move_from_const<Ptr>::type = 0);

    dynamic_move_ptr(dynamic_move_ptr&);

    template<typename TT>
    dynamic_move_ptr( dynamic_move_ptr<TT>&,
                      typename 
                      move_ptrs::enable_if_convertible<
                          TT, T, dynamic_move_ptr&
                      >::type::type* = 0 );
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
    template<typename TT>
    friend class dynamic_move_ptr;
#else
    public:
#endif
    move_ptrs::bound_deleter<is_array>  deleter_;
    element_type*                       t_;
};

template<class T>
T* get_pointer(dynamic_move_ptr<T> const& p) { return p.get(); }

} // End namespace boost.

#if defined(BOOST_MSVC)
#pragma warning(pop) // #pragma warning(disable:4251)
#endif

#endif      // #ifndef MEOW_MOVE_PTR__DYNAMIC_MOVE_PTR_HPP_
