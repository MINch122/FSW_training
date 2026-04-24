#include "ut_list_list.h"
#include "ut_list_node.h"

int main(void) {

    mlk_ut_suite_t* suite_list = mlk_ut_suite_create("suite_list");

    if (!suite_list) {
        return -1;
    }

    mlk_ut_log_attach_file(suite_list, "MLK_LIST_UT_REPORT.txt", MLK_LOG_STREAM_TEXT);
    mlk_ut_log_set_loglevel(suite_list, MLK_LOG_INFO);

    MLK_UT_ADD_TEST(suite_list, list_create);
    MLK_UT_ADD_TEST(suite_list, add_front);
    MLK_UT_ADD_TEST(suite_list, pop_front);
    MLK_UT_ADD_TEST(suite_list, pop_back);
    MLK_UT_ADD_TEST(suite_list, pop_current);

    return mlk_ut_run_suite(suite_list);

}
