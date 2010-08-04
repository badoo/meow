////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2010 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#include <meow/tree/tree.hpp>
#include <meow/tree/debug.hpp>
#include <meow/tree/path.hpp>
#include <meow/tree/path_reconstruct.hpp>

#include <meow/format/format.hpp>
#include <meow/format/sink/FILE.hpp>

#include <meow/gcc/demangle.hpp>

int main()
{
	using namespace meow::tree;
	using meow::format::fmt;

	directory_ptr root = create_directory();
	root
		->add_child("level_1_1", create_directory_p()
			->add_child("level_2_1", create_directory_p()
				->add_child("level_3_1", create_file_p<unsigned long long>(10))))
		->add_child("level_1_2", create_file_p(0))
		->add_child("level_1_3", create_directory_p()
			->add_child("level_2_2", create_file_p(meow::ref_lit("heya")))
			->add_child("level_2_3", create_directory_p()
				->add_child("level_3_2", create_file_p<size_t>())))
		;

	debug_dump(stdout, get_pointer(root));

	directory_t *r = get_pointer(root);

	{
		char const *path = "/level_1_3/level_2_3/level_3_2";
		file_t *f = get_path_as_file(r, path);
		BOOST_ASSERT(NULL != f);
		BOOST_ASSERT(f->type_info() == typeid(size_t));

		fmt(stdout, "found file: {0}, type: {1}\n", f, meow::gcc_demangle_name_tmp(f->type_info().name()));

		BOOST_ASSERT(meow::str_ref(path) == reconstruct_path_tmp(f));
		fmt(stdout, "reconstructed path: {0}\n", reconstruct_path_tmp(f));
	}

	return 0;
}


