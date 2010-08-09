////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2008 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_JUDY__JUDY_HPP_
#define MEOW_JUDY__JUDY_HPP_

extern "C" {
#include <Judy.h>
}

#include <meow/movable_handle.hpp> 	// we're doing handles here too
#include <meow/str_ref.hpp> 		// used to make def specialization for judy_HS

////////////////////////////////////////////////////////////////////////////////////////////////
namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

	typedef Word_t 		word_t;
	typedef Pvoid_t 	judy_t;

	namespace {
		int const 		j_error = 		  JERR;
		void* const		j_error_p = 	 PJERR;
		void** const	j_error_pp = 	PPJERR;
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	inline judy_t null_judy_array()
	{
		return static_cast<judy_t>(NULL);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	struct judy_traits_base
	{
		typedef judy_t handle_type;

		static handle_type null() { return handle_type(); }
		static bool is_null(handle_type const& h) { return (null() == h); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
// special case - bitset

	struct judy_traits_1 : public judy_traits_base
	{
		static void close(handle_type& h) { int r; J1FA(r, h); }
	};
	typedef meow::movable_handle<judy_traits_1> judy_1_t;

	struct judy_ops_1
	{
		typedef word_t 		key_t;
		typedef judy_1_t 	handle_t;

		static int set(judy_t& j, key_t const k) { int r; J1S(r, j, k); return r; }
		static int set(handle_t& j, key_t const k) { return set(get_handle(j), k); }

		static int unset(judy_t& j, key_t const k) { int r; J1U(r, j, k); return r; }
		static int unset(handle_t& j, key_t const k) { return unset(get_handle(j), k); }

		static int test(judy_t const& j, key_t const k) { int r; J1T(r, j, k); return r; }
		static int test(handle_t const& j, key_t const k) { return test(get_handle(j), k); }

		static int free_array(judy_t& j) { int n; J1FA(n, j); return n; }
		static int free_array(handle_t& j) { return free_array(get_handle(j)); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct judy_traits_L : public judy_traits_base
	{
		static void close(handle_type& h) { int r; JLFA(r, h); }
	};
	typedef meow::movable_handle<judy_traits_L> judy_L_t;

	struct judy_ops_L
	{
		typedef word_t 		key_t;
		typedef judy_L_t 	handle_t;

		// @desc: get index from array
		// @return
		//  NULL: not found
		//  ptr: pointer to stored value
		// @post:
		//  client has to convert the resulting pointer to apropriate type

		static void* get(judy_t const& j, key_t const k) { return (void*)JudyLGet(j, k, PJE0); }
		static void* get(handle_t const& j, key_t const k) { return get(get_handle(j), k); }

		// @desc: get or create the value with specified key
		// @return: a pointer to the inserted value
		//  j_error_p: malloc error
		//  NULL: ummm... booom
		//  ptr:
		//   *ptr == 0: value placeholder been inserted, you can change it
		//   *ptr != 0: existing value found (or you stored 0 in there)
		// @post:
		//  client has to convert the resulting pointer to apropriate type
		static void* get_or_create(judy_t& j, key_t const k) { void *v; JLI(v, j, k); return v; }
		static void* get_or_create(handle_t& j, key_t const k) { return get_or_create(get_handle(j), k); }

		// @desc: delete value with specified key
		// @return
		//  j_error: malloc fail, must not happen
		//  0: key not found
		//  1: key deleted
		static int del(judy_t& j, key_t const k) { int r; JLD(r, j, k); return r; }
		static int del(handle_t& j, key_t const k) { return del(get_handle(j), k); }

		// @desc: frees judy array
		// @return: number of bytes freed
		static int free_array(judy_t& j) { int n; JLFA(n, j); return n; }
		static int free_array(handle_t& j) { return free_array(get_handle(j)); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	struct judy_traits_SL : public judy_traits_base
	{
		static void close(handle_type& h) { int r; JSLFA(r, h); }
	};
	typedef meow::movable_handle<judy_traits_SL> judy_SL_t;

	struct judy_ops_SL
	{
		typedef uint8_t const* 	key_t;
		typedef judy_SL_t 		handle_t;

		static void* get(judy_t const& j, key_t const k) { void *v; JSLG(v, j, k); return v; }
		static void* get(handle_t const& j, key_t const k) { return get(get_handle(j), k); }

		static void* get_or_create(judy_t& j, key_t const k) { void *v; JSLI(v, j, k); return v; }
		static void* get_or_create(handle_t& j, key_t const k) { return get_or_create(get_handle(j), k); }

		static int del(judy_t& j, key_t const k) { int r; JSLD(r, j, k); return r; }
		static int del(handle_t& j, key_t const k) { return del(get_handle(j), k); }

		static int free_array(judy_t& j) { int n; JSLFA(n, j); return n; }
		static int free_array(handle_t& j) { return free_array(get_handle(j)); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T>
	struct judy_ops_HS_traits;
	/* defines the interface
	{
		// the type you specialize for
		// must be available for implementation to find it
		typedef implementation-defined key_t;

		static void* 	data(key_t const& k) {} 	// get ptr to start of key data
		static word_t 	length(key_t const& k) {} 	// length of key data in bytes
	};
	*/

	template<class T>
	struct judy_ops_HS_traits<meow::string_ref<T> >
	{
		typedef meow::string_ref<T> key_t;
		static void* 	data(key_t const& v) { return (void*)v.data(); }
		static word_t 	length(key_t const& v) { return sizeof(T) * v.length(); }
	};

	struct judy_traits_HS : public judy_traits_base
	{
		static void close(handle_type& h) { int r; JHSFA(r, h); }
	};
	typedef meow::movable_handle<judy_traits_HS> judy_HS_t;


	template<class K>
	struct judy_ops_HS
	{
		typedef K 						key_t;
		typedef judy_HS_t 				handle_t;
		typedef judy_ops_HS_traits<K> 	traits;

		static void* get(judy_t const& j, key_t const k) { void *v; JHSG(v, j, traits::data(k), traits::length(k)); return v; }
		static void* get(handle_t const& j, key_t const k) { return get(get_handle(j), k); }

		static void* get_or_create(judy_t& j, key_t const k) { void *v; JHSI(v, j, traits::data(k), traits::length(k)); return v; }
		static void* get_or_create(handle_t& j, key_t const k) { return get_or_create(get_handle(j), k); }

		static int del(judy_t& j, key_t const k) { int r; JHSD(r, j, traits::data(k), traits::length(k)); return r; }
		static int del(handle_t& j, key_t const k) { return del(get_handle(j), k); }

		static int free_array(judy_t& j) { int n; JSLFA(n, j); return n; }
		static int free_array(handle_t& j) { return free_array(get_handle(j)); }
	};

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_JUDY__JUDY_HPP_

