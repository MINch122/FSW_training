##################################################################
#
# cFE Serial Comm. (SRL) module CMake build recipe
#
# Author : HyeokJin Kweon
##################################################################

project (CFE_SRL C)
message(STATUS "srl cmake start")

# Serial Comm. source files
set(SRL_SOURCE
    fsw/src/cfe_srl_api.c
    fsw/src/cfe_srl_csp_config.c
    fsw/src/cfe_srl_csp.c
    fsw/src/cfe_srl_dispatch.c
    fsw/src/cfe_srl_handle.c
    fsw/src/cfe_srl_init.c
    fsw/src/cfe_srl_mutex.c
    fsw/src/cfe_srl_priv.c
    fsw/src/cfe_srl_task.c
)

add_library(srl STATIC ${SRL_SOURCE})

target_include_directories(srl PUBLIC fsw/inc)
target_link_libraries(srl PRIVATE core_private)
add_cfe_app_dependency(srl iodriver)

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
    # For OBC
    # link csp
    target_link_libraries(srl PRIVATE ${PROJECT_SOURCE_DIR}/../../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else ()
    # For local Linux host
    # link csp
    target_link_libraries(srl PRIVATE ${PROJECT_SOURCE_DIR}/../../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()

message(STATUS "srl cmake end")