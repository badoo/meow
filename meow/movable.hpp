////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2007 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW__MOVABLE_HPP_
#define MEOW__MOVABLE_HPP_

#include <boost/type_traits/is_base_and_derived.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace movable {
////////////////////////////////////////////////////////////////////////////////////////////////
// modeled after Andrei Alexandresku mojo protocol
// http://www.ddj.com/dept/cpp/184403855

	template<class T>
	struct constant
	{
		explicit constant(T const& p) : ptr_(&p) {}
		T const& get() const { return *ptr_; }
	private:
		T const *ptr_;
	};

	template<class T>
	struct temporary : private constant<T>
	{
		explicit temporary(T& p) : constant<T>(p) {}
		T& get() const { return const_cast<T&>(constant<T>::get()); }
	};

	template<class T>
	struct move_enabled
	{
		typedef move_enabled move_base_t;

		operator constant<T>() const { return constant<T>(static_cast<T const&>(*this)); }
		operator temporary<T>() { return temporary<T>(static_cast<T&>(*this)); }

	protected: // to be derived from
		move_enabled() {}
		~move_enabled() {}
	};

	template<class T>
	struct is_move_enabled
		: public boost::is_base_and_derived<move_enabled<T>, T>
	{
	};

	template<class T>
	temporary<T> move(T& h) { return temporary<T>(h); }

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace movable {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW__MOVABLE_HPP_

