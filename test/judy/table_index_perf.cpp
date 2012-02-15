////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////
//
// cd meow/test/judy/
// g++ -O0 -g3 -I /opt/local/include/ -I ~/_Dev/meow/ -I ~/_Dev/_libs/boost/1.47.0 -o table_index_perf table_index_perf.cpp /opt/local/lib/libJudy.a
//

#include <meow/judy/table_index.hpp>
//#include <meow/hash/hash.hpp>
#include <meow/hash/hash_jenkins3.hpp>
#include <meow/hash/hash_murmur3.hpp>
#include <meow/str_ref.hpp>
#include <meow/str_copy.hpp>

using namespace meow;

struct test_t
{
	uint32_t        id;
	meow::str_copy  name;
};

struct hash_jenkins_t
{
	size_t operator()(str_copy const& k) const
	{
		str_ref const& kr = k.ref();
		return hash_impl<hash_jenkins_tag>::hash_blob(kr.data(), kr.c_length(), 0);
	}
};

struct hash_murmur3_t
{
	size_t operator()(str_copy const& k) const
	{
		str_ref const& kr = k.ref();
		return hash_impl<hash_murmur3_tag>::hash_blob(kr.data(), kr.c_length(), 0);
	}
};

struct test_equal_t
{
	bool operator()(uint32_t const k, test_t const& v) const
	{
		return k == v.id;
	}

	bool operator()(str_copy const& k, test_t const& v) const
	{
		return k == v.name;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/foreach.hpp>
#include <meow/stopwatch.hpp>
//#include <meow/judy/table_index_debug.hpp>

#include <meow/format/format.hpp>
namespace ff = meow::format;

void* operator new(std::size_t sz)
{
	void *p = ::malloc(sz);
	if (NULL == p)
		throw std::bad_alloc();
	return p;
}

void operator delete(void *p)
{
	::free(p);
}

typedef std::vector<test_t> test_vector_t;

void bench_judysl(test_vector_t const& v)
{
	judy::judy_t j_ = NULL;
	typedef judy::judy_select<str_ref>::operations_t j_ops;
	{
		stopwatch_t sw;
		ff::fmt(stdout, "{0}; inserting started: {1}\n", __func__, sw.now());
		BOOST_FOREACH(test_t const& t, v)
		{
			test_t **p = (test_t **)j_ops::get_or_create(j_, t.name.ref());
			*p = (test_t*)&t;
		}
		ff::fmt(stdout, "{0}; inserting done: {1}, elapsed: {2}\n", __func__, sw.now(), sw.stamp());
	}

	{
		stopwatch_t sw;
		ff::fmt(stdout, "{0}; searching started: {1}\n", __func__, sw.now());
		BOOST_FOREACH(test_t const& t, v)
		{
			test_t **p = (test_t **)j_ops::get(j_, t.name.ref());
		}
		ff::fmt(stdout, "{0}; searching done: {1}, elapsed: {2}\n", __func__, sw.now(), sw.stamp());
	}
}

template<class HashT>
void bench_table_index(test_vector_t const& v)
{
	typedef judy::table_index_t<
						  test_t
						, HashT
						, test_equal_t
						>
						index_t;

	index_t idx;

	{
		stopwatch_t sw;
		ff::fmt(stdout, "{0}; inserting started: {1}\n", __PRETTY_FUNCTION__, sw.now());
		BOOST_FOREACH(test_t const& t, v)
		{
			test_t *tt = (test_t*)&t;
			test_t **p = idx.get_or_create(t.name, tt);
		}
		ff::fmt(stdout, "{0}; inserting done: {1}, elapsed: {2}\n", __PRETTY_FUNCTION__, sw.now(), sw.stamp());
	}

	{
		stopwatch_t sw;
		ff::fmt(stdout, "{0}; searching started: {1}\n", __PRETTY_FUNCTION__, sw.now());
		BOOST_FOREACH(test_t const& t, v)
		{
			test_t **p = idx.get(t.name);
		}
		ff::fmt(stdout, "{0}; searching done: {1}, elapsed: {2}\n", __PRETTY_FUNCTION__, sw.now(), sw.stamp());
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		ff::fmt(stdout, "usage: {0} <file with strings>\n", argv[0]);
		return 1;
	}

	char const *filename = argv[1];
	FILE *f = fopen(filename, "r");

	std::vector<test_t> v;

	int i = 0;
	while (!feof(f))
	{
		char buf[1024];
		if (NULL == fgets(buf, sizeof(buf), f))
			continue;

		int len = strlen(buf);
		if ('\n' == buf[len - 1])
		{
			if ('\r' == buf[len - 2])
				len -= 2;
			else
				len -= 1;

			buf[len] = 0x0;
		}

		{
			test_t t;
			t.id = i++;
			t.name = str_ref(buf, len+1);
			v.push_back(t);
		}

		//insert_kv(&idx, i++, str_ref(buf, len));
	}

	bench_judysl(v);
	bench_table_index<hash_jenkins_t>(v);
	bench_table_index<hash_murmur3_t>(v);

//	report_index_state(idx, stdout);
}

