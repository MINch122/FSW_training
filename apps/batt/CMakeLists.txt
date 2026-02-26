project(CFE_BATT_APP C)

set(APP_SRC_FILES
  fsw/src/batt_app.c
  fsw/src/batt_cmds.c
  fsw/src/batt_dispatch.c
)

# Create the app module
add_cfe_app(batt ${APP_SRC_FILES})

target_include_directories(batt PUBLIC
  fsw/inc
  ../../cfe/modules/srl/fsw/src
  ../../submodules/libgscsp/lib/libftp_client/include
  ../../submodules/libgscsp/lib/libgscsp/include
  ../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include
  ../../submodules/libgscsp/lib/libparam_client/include
  ../../submodules/libgscsp/lib/libparam_client/include/deprecated
  ../../submodules/libgscsp/lib/libutil/include
  ../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include
)

target_compile_options(batt PRIVATE -Wno-pedantic -Wno-error=pedantic)
target_compile_definitions(batt PRIVATE GS_PARAM_INTERNAL_USE=1)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    # OBC build
    target_link_libraries(batt ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
    # CPU1 build
    target_link_libraries(batt ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()
