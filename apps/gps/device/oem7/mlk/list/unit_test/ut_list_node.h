#ifndef _UT_LIST_NODE_H_
#define _UT_LIST_NODE_H_

#include <mlk_list.h>
#include <mlk_ut.h>

MLK_UT_DEF_TEST(add_front) {
    mlk_list_t list = mlk_list();
    int d;

    mlk_ut_assert_ptr_nonnull(list);

    for (int i = 1; i < 5; i++) {
        mlk_ut_assert_int_eq(mlk_list_add_front(list, &i, sizeof(i)), 0);

        mlk_ut_assert_int_eq(mlk_list_nodes(list), i);
        mlk_ut_assert_int_eq(mlk_list_node_size(list), sizeof(i));
        mlk_ut_assert_ptr_nonnull(mlk_list_node_data(list));
        mlk_ut_assert_ptr_neq(mlk_list_node_data(list), &i);

        mlk_list_to_head(list);
        mlk_ut_assert_int_eq(mlk_list_node_dump(list, &d), 0);
        mlk_ut_assert_int_eq(d, i);
    }

    // mlk_ut_assert_true(mlk_list_is_tail(list));

    mlk_list_to_head(list);
    mlk_ut_assert_int_eq(mlk_list_node_dump(list, &d), 0);
    mlk_ut_assert_int_eq(d, 4);
    mlk_list_to_tail(list);
    mlk_ut_assert_int_eq(mlk_list_node_dump(list, &d), 0);
    mlk_ut_assert_int_eq(d, 1);

    mlk_list_free(list);
}

// MLK_UT_DEF_TEST(node_back) {

// }

// MLK_UT_DEF_TEST(node_insert) {

// }

// MLK_UT_DEF_TEST(node_ptr) {

// }

MLK_UT_DEF_TEST(pop_front) {
    mlk_list_t list = mlk_list();
    int k;
    mlk_ut_assert_ptr_nonnull(list);

    // 1. Can't pop on an empty list.
    mlk_ut_assert_int_neq(mlk_list_pop_front(list, NULL), 0);
    
    // 2. Single node pop test.
    mlk_list_add_front(list, &k, sizeof(k));
    mlk_ut_assert_int_eq(mlk_list_pop_front(list, &k), 0);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 0);

    // 3. Pops on multiple nodes and data check.
    for (int i = 1; i <= 5; i++) {
        mlk_list_add_front(list, &i, sizeof(i));
    }
    mlk_list_to_head(list);
    for (int i = 5; i >= 1; i--) {
        mlk_ut_assert_int_eq(mlk_list_pop_front(list, &k), 0);
        mlk_ut_assert_int_eq(mlk_list_nodes(list), i-1);
        // Front-pop should maintain the accessing node previously at head.
        if (i > 1)
            mlk_ut_assert_true(mlk_list_is_head(list));
        mlk_ut_assert_int_eq(k, i);
    }
    mlk_list_free(list);
}

MLK_UT_DEF_TEST(pop_back) {
    mlk_list_t list = mlk_list();
    int k;
    mlk_ut_assert_ptr_nonnull(list);

    // 1. Can't pop on an empty list.
    mlk_ut_assert_int_neq(mlk_list_pop_back(list, NULL), 0);
    
    // 2. Single node pop test.
    mlk_list_add_front(list, &k, sizeof(k));
    mlk_ut_assert_int_eq(mlk_list_pop_back(list, &k), 0);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 0);

    // 3. Pops on multiple nodes and data check.
    for (int i = 1; i <= 5; i++) {
        mlk_list_add_front(list, &i, sizeof(i));
    }

    for (int i = 1; i <= 5; i++) {
        mlk_ut_assert_int_eq(mlk_list_pop_back(list, &k), 0);
        mlk_ut_assert_int_eq(mlk_list_nodes(list), 5-i);
        // Pop-back should maintain the accessing node at tail.
        if (i < 5)
            mlk_ut_assert_true(mlk_list_is_tail(list));
        mlk_ut_assert_int_eq(k, i);
    }
    mlk_list_free(list);
}

MLK_UT_DEF_TEST(pop_current) {
    mlk_list_t list = mlk_list();
    int k;
    mlk_ut_assert_ptr_nonnull(list);

// 1. Can't pop on an empty list.
    mlk_ut_assert_int_neq(mlk_list_pop_current(list, NULL), 0);
    
// 2. Single node pop test.
    mlk_list_add_front(list, &k, sizeof(k));
    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 0);

// 3. Pops on multiple nodes and data check.
    for (int i = 1; i <= 10; i++) {
        // 1 2 3 4 5 6 7 8 9 10
        mlk_list_add_back(list, &i, sizeof(i));
    }

    mlk_list_to_head(list);

// 3-1. Head pop preserves the accessing node at head.
    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    // 2 3 4 5 6 7 8 9 10
    mlk_ut_assert_int_eq(k, 1);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 9);
    mlk_ut_assert_true(mlk_list_is_head(list));

    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    // 3 4 5 6 7 8 9 10
    mlk_ut_assert_int_eq(k, 2);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 8);
    mlk_ut_assert_true(mlk_list_is_head(list));

// 3-2. Pop in the middle sets the accessing node to next.
    mlk_list_advance(list);
    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    // 3 5 6 7 8 9 10
    mlk_ut_assert_int_eq(k, 4);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 7);
    mlk_ut_assert_false(mlk_list_is_head(list));

    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    // 3 6 7 8 9 10
    mlk_ut_assert_int_eq(k, 5);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 6);
    mlk_ut_assert_false(mlk_list_is_head(list));

// 3-3. Tail pop preserves the accessing node at tail.
    mlk_list_to_tail(list);
    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    // 3 6 7 8 9
    mlk_ut_assert_int_eq(k, 10);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 5);
    mlk_ut_assert_true(mlk_list_is_tail(list));

    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    // 3 6 7 8
    mlk_ut_assert_int_eq(k, 9);
    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    mlk_ut_assert_true(mlk_list_is_tail(list));
    // 3 6 7
    mlk_ut_assert_int_eq(k, 8);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 3);
    mlk_ut_assert_true(mlk_list_is_tail(list));

// 3-4. Subsequent middle-pop precedence.
    mlk_list_to_head(list);
    mlk_list_advance(list);
    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    // 3 7
    mlk_ut_assert_int_eq(k, 6);
    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    // 3
    mlk_ut_assert_int_eq(k, 7);
    mlk_ut_assert_int_eq(mlk_list_pop_current(list, &k), 0);
    // none.
    mlk_ut_assert_int_eq(k, 3);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 0);
    mlk_ut_assert_int_neq(mlk_list_pop_current(list, &k), 0);
    mlk_ut_assert_int_eq(mlk_list_nodes(list), 0);

    mlk_list_free(list);

}


#endif