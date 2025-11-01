#ifndef _UT_LIST_LIST_H_
#define _UT_LIST_LIST_H_

#include <mlk_list.h>
#include <mlk_ut.h>

MLK_UT_DEF_TEST(list_create) {
    mlk_list_t list = mlk_list();

    mlk_ut_assert_ptr_nonnull(list);

    mlk_ut_assert_int_eq(mlk_list_nodes(list), 0);
    mlk_ut_assert_int_eq(mlk_list_node_size(list), 0);
    mlk_ut_assert_ptr_null(mlk_list_node_data(list));

    mlk_ut_assert_false(mlk_list_to_head(list));
    mlk_ut_assert_false(mlk_list_is_tail(list));
    mlk_ut_assert_false(mlk_list_advance(list));
}



#endif