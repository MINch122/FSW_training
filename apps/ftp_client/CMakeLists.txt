project(CFE_FTP_CLIENT C)

set(APP_SRC_FILES
  fsw/src/ftp_client_app.c
  fsw/src/ftp_client_cmds.c
  fsw/src/ftp_client_dispatch.c
)

add_cfe_app(ftp_client ${APP_SRC_FILES})

target_include_directories(ftp_client PUBLIC
  fsw/inc
  fsw/src
  ../../submodules/libgscsp/lib/libftp_client/include
  ../../submodules/libgscsp/lib/libgscsp/include
  ../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include
  ../../submodules/libgscsp/lib/libutil/include
  ../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include
)

target_compile_options(ftp_client PRIVATE -Wno-pedantic -Wno-error=pedantic)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  target_link_libraries(ftp_client ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
  target_link_libraries(ftp_client ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()
