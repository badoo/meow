////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////
//
// cd meow/test/judy/
// g++ -O0 -g3 -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.41.0 -o iteration_test iteration_test.cpp -L/opt/local/lib -lJudy
//

#include <meow/format/format.hpp>
#include <meow/format/sink/FILE.hpp>

#include <meow/judy/index.hpp>

namespace ff = meow::format;


struct test_t
{
	int a;
	test_t(int aa) : a(aa) {}
};

typedef judy::index_t<int, test_t*> index_t;

void insert_value(index_t& idx, int key)
{
	test_t *t = new test_t(key);
	ff::fmt(stdout, "t: {0}, {1}\n", t, t->a);
	test_t **node = idx.get_or_create(t->a);
	*node = t;
}

void print_kv(int k, void *vv)
{
	test_t *v = (test_t*)vv;
	ff::fmt(stdout, "{0} -> {1} {{ {2}, 0x{3} }\n", k, v, v->a, ff::as_hex(v->a));
}

int main()
{
	index_t idx;
	insert_value(idx, 1);
	insert_value(idx, 10);
	insert_value(idx, 5);
	insert_value(idx, 21);
	insert_value(idx, -100);
	insert_value(idx, 0);

	idx.for_each(&print_kv);
}

