////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_FORMAT__METAFUNCTIONS_HPP_
#define MEOW_FORMAT__METAFUNCTIONS_HPP_

#include <cstddef>
#include <meow/str_ref.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T, class Enabler = void>
	struct string_access
	{
		inline static T const& call(T const& v) { return v; }
	};

	template<class T, class Enabler = void>
	struct type_tunnel
	{
		inline static T const& call(T const& v) { return v; }
	};

	template<class T>
	inline auto tunnel(T const& v) -> decltype(type_tunnel<T>::call(v))
	{
		return type_tunnel<T>::call(v);
	}

////////////////////////////////////////////////////////////////////////////////////////////////

	template<class T, class Enabler = void>
	struct sink_write
	{
		template<class CharT>
		inline static void call(T& sink, size_t total_len, string_ref<CharT> const *slices, size_t n_slices)
		{
			sink.write(total_len, slices, n_slices);
		}
	};

	template<class SinkT, class CharT>
	inline SinkT& write_to_sink(SinkT& sink, size_t l, string_ref<CharT> const *slices, size_t n_slices)
	{
		sink_write<SinkT>::call(sink, l, slices, n_slices);
		return sink;
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace format {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_FORMAT__METAFUNCTIONS_HPP_

