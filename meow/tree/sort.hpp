////////////////////////////////////////////////////////////////////////////////////////////////
// vim: set filetype=cpp autoindent noexpandtab tabstop=4 shiftwidth=4 foldmethod=marker :
// (c) 2011 Anton Povarov <anton.povarov@gmail.com>
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MEOW_TREE__SORT_HPP_
#define MEOW_TREE__SORT_HPP_

#include <algorithm> // std::sort, std::less, std::greater

#include <meow/std_bind.hpp>
#include <meow/tree/tree.hpp>
#include <meow/tree/for_each.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////
namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

	namespace tree_sort_detail_
	{
		struct sorter_t
		{
			typedef void result_type;

			template<class CompareT>
			result_type operator()(CompareT const& comp, directory_t *d, str_ref const&, int) const
			{
				directory_t::child_range_nc_t r = d->get_children();
				std::sort(r.begin(), r.end(), comp);
			}

			static result_type on_file_noop(file_t*, str_ref const&, int)
			{
			}
		};
	}

	template<template<class> class OperationT>
	struct tree_node_comp_by_type_t
	{
		OperationT<node_type_t> op_;

		bool operator()(directory_t::child_t const& l, directory_t::child_t const& r) const
		{
			return op_(l.ptr->type(), r.ptr->type());
		}
	};

	typedef tree_node_comp_by_type_t<std::less> node_type_comp__files_first_t;
	typedef tree_node_comp_by_type_t<std::greater> node_type_comp__dirs_first_t;

	template<class CompareT>
	void tree_sort(directory_t *d, CompareT const& comp)
	{
		tree_for_each(
				  d
				, std::bind(tree_sort_detail_::sorter_t(), std::cref(comp), _1, _2, _3)
				, &tree_sort_detail_::sorter_t::on_file_noop
				);
	}

////////////////////////////////////////////////////////////////////////////////////////////////
}} // namespace meow { namespace tree {
////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MEOW_TREE__SORT_HPP_

