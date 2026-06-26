cmake_minimum_required(VERSION 2.6.4)
project(CFS_FTP_ORIGIN C)

include_directories(fsw/inc)
include_directories(${io_lib_MISSION_DIR}/public_inc)
include_directories(../io_lib/fsw/public_inc)

aux_source_directory(fsw/src APP_SRC_FILES)

# Link CSP
include_directories(../../submodules/libgscsp/lib/libgattos-uc_client_client/include)
include_directories(../../submodules/libgscsp/lib/isl-modem-app_client/include)
include_directories(../../submodules/libgscsp/lib/libaardvark/include)
include_directories(../../submodules/libgscsp/lib/libadcs_client/include)
include_directories(../../submodules/libgscsp/lib/libfp_client/include)
include_directories(../../submodules/libgscsp/lib/libftp_client/include)
include_directories(../../submodules/libgscsp/lib/libgosh_client/include)
include_directories(../../submodules/libgscsp/lib/libgscsp/include)
include_directories(../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include)
include_directories(../../submodules/libgscsp/lib/libgscsp/lib/libcsp/src) # #include <csp_conn.h>
include_directories(../../submodules/libgscsp/lib/libgssb_client/include)
include_directories(../../submodules/libgscsp/lib/libhk_client/include)
include_directories(../../submodules/libgscsp/lib/libnanopb_client/include)
include_directories(../../submodules/libgscsp/lib/libp60_client/include)
include_directories(../../submodules/libgscsp/lib/libparam_client/include)
include_directories(../../submodules/libgscsp/lib/librgosh_client/include)
include_directories(../../submodules/libgscsp/lib/libutil/include)
include_directories(../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include)

# Create the app module
add_cfe_app(ftp_origin ${APP_SRC_FILES})
target_compile_options(ftp_origin PRIVATE -Wno-pedantic -Wno-error=pedantic)

if (CMAKE_SYSTEM_PROCESSOR	STREQUAL "arm")
    # For OBC (cross-compiled libgpiod)
    # link csp
    target_link_libraries(ftp_origin ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else ()
    # For local Linux host
    # link csp
    target_link_libraries(ftp_origin ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()
