project(CFE_MEOW C)

set(APP_SRC_FILES
  fsw/src/meow.c
  fsw/src/meow_cmds.c
  fsw/src/core/meow_shell.c
  fsw/src/core/meow_file.c
  fsw/src/core/meow_sys.c
  fsw/src/core/meow_csp.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/meow_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/meow_dispatch.c
  )
endif()

add_cfe_app(meow ${APP_SRC_FILES})

target_include_directories(meow PUBLIC  fsw/inc)
target_include_directories(meow PRIVATE fsw/src)
target_include_directories(meow PRIVATE fsw/src/core)
target_include_directories(meow PRIVATE
  ../../submodules/libgscsp/lib/libftp_client/include
  ../../submodules/libgscsp/lib/libgscsp/include
  ../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include
  ../../submodules/libgscsp/lib/libgscsp/lib/libcsp/src
  ../../submodules/libgscsp/lib/libutil/include
  ../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include
)

target_compile_options(meow PRIVATE -Wno-pedantic -Wno-error=pedantic)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  target_link_libraries(meow ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
  target_link_libraries(meow ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()

if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
