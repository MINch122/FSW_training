##################################################################
#
# cFE Radio Comm. (RF) module CMake build recipe
#
# Author : HyeokJin Kweon
##################################################################

project (CFE_RF C)

# Unit Test stub flags
set(ENABLE_UT TRUE)

# Serial Comm. source files
if (ENABLE_UT)
    set(rf_SOURCES
        fsw/src/cfe_rf_api.c
        # fsw/src/cfe_srl_basic.c
        # fsw/src/cfe_srl_io_stub.c
        # fsw/src/cfe_srl_mutex.c
        # fsw/src/cfe_srl_handle.c
        # fsw/src/cfe_srl_priv.c
        # fsw/src/cfe_srl_csp.c
    )
else()
    set(rf_SOURCES
        fsw/src/cfe_rf_api.c
    # fsw/src/cfe_srl_basic.c
    # fsw/src/cfe_srl_io.c
    # fsw/src/cfe_srl_mutex.c
    # fsw/src/cfe_srl_handle.c
    # fsw/src/cfe_srl_priv.c
    # fsw/src/cfe_srl_csp.c
    )   
endif()

add_library(rf STATIC ${rf_SOURCES})

target_include_directories(rf PUBLIC fsw/inc)
target_link_libraries(rf PRIVATE core_private)

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

target_compile_options(rf PRIVATE -Wno-pedantic -Wno-error=pedantic)

if (CMAKE_SYSTEM_PROCESSOR	STREQUAL "arm")
    # For OBC (cross-compiled libgpiod)
    # link csp
    target_link_libraries(rf PRIVATE ${PROJECT_SOURCE_DIR}/../../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else ()
    # For local Linux host
    # link csp
    target_link_libraries(rf PRIVATE ${PROJECT_SOURCE_DIR}/../../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()