cmake_minimum_required(VERSION 3.16)
project(CFE_SANT_APP C)

message(STATUS "sant_app cmake start")

set(APP_SRC_FILES
    fsw/src/sant_app.c
    fsw/src/sant_cmds.c
    fsw/src/sant_utils.c
    fsw/src/sant_dispatch.c
)

add_cfe_app(sant ${APP_SRC_FILES})

target_include_directories(sant PUBLIC
    fsw/inc
    fsw/src
    ../../submodules/libgscsp/lib/libutil/include/
    ../../submodules/libgscsp/lib/libgssb_client/include/
)

target_compile_options(sant PRIVATE -Wno-error=pedantic)

if(CMAKE_SYSTEM_PROCESSOR	STREQUAL "arm")
    # OBC build
    target_link_libraries(sant ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
    # CPU1 build
    target_link_libraries(sant ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()

message(STATUS "sant_app cmake end")