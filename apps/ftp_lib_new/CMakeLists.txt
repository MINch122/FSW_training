project(CFE_FTP_LIB_NEW C)

set(APP_SRC_FILES
  server/src/ftpnew.c
  server/src/ftpnew_utils.c
)

add_cfe_app(ftp_lib_new ${APP_SRC_FILES})

target_include_directories(ftp_lib_new PUBLIC
  server/inc
  config
  test/external/include
  ../../submodules/libgscsp/lib/libftp/include
  ../../submodules/libgscsp/lib/libftp_client/include
  ../../submodules/libgscsp/lib/libgscsp/include
  ../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include
  ../../submodules/libgscsp/lib/libgscsp/lib/libcsp/src
  ../../submodules/libgscsp/lib/libutil/include
  ../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include
)

target_compile_options(ftp_lib_new PRIVATE -Wno-pedantic -Wno-error=pedantic -Wno-error=strict-prototypes -Wno-strict-prototypes)
target_link_libraries(ftp_lib_new pthread)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  target_link_libraries(ftp_lib_new ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
  target_link_libraries(ftp_lib_new ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()
