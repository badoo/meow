// (C) Copyright Jonathan Turkanis.
// (C) Copyright Daniel Wallin.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// Contains the definition of the class template bound_deleter, used to 
// implement stateless deleters for dynamic_move_ptr, yielding an interface 
// similar to boost::shared_ptr.

#ifndef BOOST_MOVE_PTR_BOUND_DELETER_HPP_INCLUDED
#define BOOST_MOVE_PTR_BOUND_DELETER_HPP_INCLUDED

#include <boost/checked_delete.hpp> 
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_array.hpp>
#include <boost/type_traits/remove_bounds.hpp>

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4521)        // Multiple copy constuctors.
#endif

namespace boost { 

namespace move_ptrs {

// Used to generate a static function which deletes a void* by
// casting it to a T* and passing it to an instance of a stateless 
// deleter type D.
template<typename T, typename D>
struct deleter_generator {
    static void delete_(const volatile void* pv) 
        { 
            D()((T*)(pv)); 
        }
};

// Consists of a deletion function together with a void*.
template<bool IsArray>
struct bound_deleter {
    bound_deleter() : fn_(0), pv_(0) { }
    template<typename T>
    bound_deleter(T* t) : pv_(t)
        {
            typedef typename 
                    mpl::if_c<
                        IsArray, 
                        checked_array_deleter<T>,
                        checked_deleter<T>
                    >::type deleter;
            fn_ = deleter_generator<T, deleter>::delete_;
        }
    template<typename T, typename D>
    bound_deleter(T* t, D d) 
        : fn_(deleter_generator<T, D>::delete_), pv_(t)
        { }
    bound_deleter(const bound_deleter& d) 
        : fn_(d.fn_), pv_(d.pv_)
        { }
    void delete_() const { fn_(pv_); }
    typedef void (*deleter_function)(const volatile void*);
    deleter_function  fn_;
    void*             pv_;
};

} } // End namespaces move_ptr, boost.

#endif // #ifndef BOOST_MOVE_PTR_BOUND_DELETER_HPP_INCLUDED
