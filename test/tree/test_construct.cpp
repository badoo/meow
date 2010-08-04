////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#include <meow/tree/tree.hpp>
#include <meow/tree/debug.hpp>

#include <meow/format/format.hpp>
#include <meow/format/sink/FILE.hpp>

struct test_t
{
	int a;
	test_t *ptr;
};

int main()
{
	using namespace meow::tree;

	directory_ptr root = create_directory();
	root
		->add_child("level_1_1", create_directory_p()
			->add_child("level_2_1", create_directory_p()
				->add_child("level_3_1", create_file_p<unsigned long long>(10))))
		->add_child("level_1_2", create_file_p(0))
		->add_child("level_1_3", create_directory_p()
			->add_child("level_2_2", create_file_p(meow::ref_lit("heya")))
			->add_child("level_2_3", create_directory_p()
				->add_child("level_3_2", create_file_p<size_t>())
				->add_child("level_3_3", create_file_p<test_t>())))
		;

	debug_dump(stdout, get_pointer(root));

	return 0;
}

