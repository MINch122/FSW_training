cmake_minimum_required(VERSION 2.6.4)
project(CFE_FTP_NEW C)

include_directories(server/inc)
include_directories(config)
include_directories(test/external/include)

include_directories(../../submodules/libgscsp/lib/libftp/include)
include_directories(../../submodules/libgscsp/lib/libftp_client/include)
include_directories(../../submodules/libgscsp/lib/libgscsp/include)
include_directories(../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include)
include_directories(../../submodules/libgscsp/lib/libgscsp/lib/libcsp/src)
include_directories(../../submodules/libgscsp/lib/libutil/include)
include_directories(../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include)

set(APP_SRC_FILES
    server/src/ftpnew.c
    server/src/ftpnew_utils.c
)

add_cfe_app(ftp_new_server ${APP_SRC_FILES})
target_compile_options(ftp_new_server PRIVATE
    -Wno-pedantic
    -Wno-error=pedantic
    -Wno-error=strict-prototypes
    -Wno-strict-prototypes
)

target_link_libraries(ftp_new_server pthread)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    target_link_libraries(ftp_new_server ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else ()
    target_link_libraries(ftp_new_server ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()
