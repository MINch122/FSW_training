add_executable(mlk_ut_list ut_list.c)

target_include_directories(mlk_ut_list
                           PUBLIC ${MODULE_SRC_DIR_ut}/inc
                                  ${MODULE_SRC_DIR_list}/inc
)

target_link_libraries(mlk_ut_list mlk)
install(TARGETS mlk_ut_list DESTINATION "${INSTALLROOT}/ut/list")