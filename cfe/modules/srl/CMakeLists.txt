##################################################################
#
# cFE Serial Comm. (SRL) module CMake build recipe
#
# Author : HyeokJin Kweon
##################################################################

project (CFE_SRL C)
message(STATUS "srl cmake start")

# Serial Comm. source files
if (ENABLE_UT)
    set(srl_SOURCES
        fsw/src/cfe_srl_basic.c
        fsw/src/cfe_srl_io_stub.c
        fsw/src/cfe_srl_mutex.c
        fsw/src/cfe_srl_handle.c
        fsw/src/cfe_srl_priv.c
        fsw/src/cfe_srl_csp.c
        fsw/src/cfe_srl_csp_config.c
        fsw/src/cfe_srl_api.c
        fsw/src/cfe_srl_init.c
        fsw/src/cfe_srl_task.c
        fsw/src/cfe_srl_dispatch.c
    )
else()
    set(srl_SOURCES
    fsw/src/cfe_srl_basic.c
    fsw/src/cfe_srl_io.c
    fsw/src/cfe_srl_mutex.c
    fsw/src/cfe_srl_handle.c
    fsw/src/cfe_srl_priv.c
    fsw/src/cfe_srl_csp.c
    fsw/src/cfe_srl_csp_config.c
    fsw/src/cfe_srl_api.c
    fsw/src/cfe_srl_init.c
    fsw/src/cfe_srl_task.c
    fsw/src/cfe_srl_dispatch.c
    )   
endif()

add_library(srl STATIC ${srl_SOURCES})

target_include_directories(srl PUBLIC fsw/inc)
target_link_libraries(srl PRIVATE core_private)

# Link CSP
include_directories(../../../submodules/libgscsp/lib/libgattos-uc_client_client/include)
include_directories(../../../submodules/libgscsp/lib/isl-modem-app_client/include)
include_directories(../../../submodules/libgscsp/lib/libaardvark/include)
include_directories(../../../submodules/libgscsp/lib/libadcs_client/include)
include_directories(../../../submodules/libgscsp/lib/libfp_client/include)
include_directories(../../../submodules/libgscsp/lib/libftp_client/include)
include_directories(../../../submodules/libgscsp/lib/libgosh_client/include)
include_directories(../../../submodules/libgscsp/lib/libgscsp/include)
include_directories(../../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include)
include_directories(../../../submodules/libgscsp/lib/libgscsp/lib/libcsp/src) # #include <csp_conn.h>
include_directories(../../../submodules/libgscsp/lib/libgssb_client/include)
include_directories(../../../submodules/libgscsp/lib/libhk_client/include)
include_directories(../../../submodules/libgscsp/lib/libnanopb_client/include)
include_directories(../../../submodules/libgscsp/lib/libp60_client/include)
include_directories(../../../submodules/libgscsp/lib/libparam_client/include)
include_directories(../../../submodules/libgscsp/lib/librgosh_client/include)
include_directories(../../../submodules/libgscsp/lib/libutil/include)
include_directories(../../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include)

target_compile_options(srl PRIVATE -Wno-pedantic -Wno-error=pedantic)

message(STATUS "Processor = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")


message(STATUS "GPIOD LIB: ${PROJECT_SOURCE_DIR}/../../../submodules/libgpiod/obc/lib/libgpiod.a")
message(STATUS "GPIOD INCLUDE: ${PROJECT_SOURCE_DIR}/../../../submodules/libgpiod/obc/include")

if (CMAKE_SYSTEM_PROCESSOR	STREQUAL "arm")
    # For OBC (cross-compiled libgpiod)
    # gpiod
    target_include_directories(srl PRIVATE ${PROJECT_SOURCE_DIR}/../../../submodules/libgpiod/obc/include)
    target_link_libraries(srl PRIVATE ${PROJECT_SOURCE_DIR}/../../../submodules/libgpiod/obc/lib/libgpiod.a)
    # link csp
    target_link_libraries(srl PRIVATE ${PROJECT_SOURCE_DIR}/../../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else ()
    # For local Linux host
    # link libgpiod.so -> drop this
    #find_library(GPIOD_LIB gpiod REQUIRED)
    # Must enter the keyword `PRIVATE`
    #target_link_libraries(srl PRIVATE ${GPIOD_LIB})

    # link libgpiod.a
    target_include_directories(srl PRIVATE /usr/include)
    target_link_libraries(srl PRIVATE /usr/lib/x86_64-linux-gnu/libgpiod.a)

    # link csp
    target_link_libraries(srl PRIVATE ${PROJECT_SOURCE_DIR}/../../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()

message(STATUS "srl cmake end")