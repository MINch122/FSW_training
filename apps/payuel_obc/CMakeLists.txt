project(CFE_PAYUEL_OBC C)

set(APP_SRC_FILES
  fsw/src/payuel_obc.c
  fsw/src/payuel_obc_child.c
  fsw/src/payuel_obc_cmds.c
  fsw/src/payuel_obc_utils.c
  fsw/src/payuel_obc_dispatch.c
)

add_cfe_app(payuel_obc ${APP_SRC_FILES})

target_include_directories(payuel_obc PUBLIC fsw/inc)

target_include_directories(payuel_obc PRIVATE
    ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/lib/libgscsp/include
    ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/lib/libgscsp/include/gs/csp
    ${PROJECT_SOURCE_DIR}/../../cfe/modules/srl/fsw/src
)

target_compile_options(payuel_obc PRIVATE -Wno-pedantic -Wno-error=pedantic)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    target_link_libraries(payuel_obc ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
    target_link_libraries(payuel_obc ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()

add_cfe_tables(payuel_obc fsw/tables/payuel_obc_tbl.c)
