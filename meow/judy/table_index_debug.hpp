////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2009 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_JUDY__TABLE_INDEX_DEBUG_HPP_
#define MEOW_JUDY__TABLE_INDEX_DEBUG_HPP_

#include <cstdio>

#include "judy.hpp"
#include "table_index.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////
namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

	template<class V, class H, class E, size_t A>
	void report_index_state(table_index_t<V, H, E, A> const& idx, FILE *to_file = stdout)
	{
		typedef table_index_t<V, H, E, A> index_t;

		judy::judy_t jj = get_handle(idx);
		fprintf(to_file, "%s\n", __PRETTY_FUNCTION__);
		fprintf(to_file, " items: %zu, memory_used: %zu\n", JudyLCount(jj, 0, -1, PJE0), JudyLMemUsed(jj));

		typedef typename index_t::leaf_t 			leaf_t;
		typedef typename leaf_t::rich_pointer_t 	rptr_t;

		size_t i = 0;
		for (leaf_t *l = (leaf_t*)JudyLFirst(jj, &i, PJE0); l != NULL; l = (leaf_t*)JudyLNext(jj, &i, PJE0))
		{
			fprintf(to_file, " leaf_p: %p; ptr: %p, type: %d; ", l, l->raw_ptr(), (int)l->type());

			typedef typename leaf_t::value_range_t range_t;
			range_t const r = l->decode_value_range();

			fprintf(to_file, "-> [%zu] { ", r.size());
			for (size_t i = 0; i < r.size(); ++i)
			{
				if (0 != i)
					printf(", ");
				fprintf(to_file, "%p", *(r.begin() + i));
			}
			fprintf(to_file, " }");
			fprintf(to_file, "\n");
		}
		fprintf(to_file, "END\n");
	}

////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace judy {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_JUDY__TABLE_INDEX_DEBUG_HPP_

