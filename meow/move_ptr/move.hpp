// (C) Copyright Daniel Wallin 2004.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// Contains the definitions of the class template move_source and the function
// template move, which together make move pointers moveable.

#ifndef MEOW_MOVE_PTR__MOVE_HPP_
#define MEOW_MOVE_PTR__MOVE_HPP_

namespace boost {

namespace move_ptrs {

template<typename Ptr>
class move_source {
public:
    move_source(Ptr& ptr) : ptr_(ptr) {}
    Ptr& ptr() const { return ptr_; }
private:
    Ptr& ptr_;
    move_source(const Ptr&);
};

} // End namespace move_ptrs.

template<typename T>
move_ptrs::move_source<T> move(T& x) 
{ return move_ptrs::move_source<T>(x); }

} // End namespace boost.

#endif // #ifndef MEOW_MOVE_PTR__MOVE_HPP_

