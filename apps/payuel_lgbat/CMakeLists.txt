project(CFE_LGBAT_APP C)

set(APP_SRC_FILES
  fsw/src/lgbat_task.c
  fsw/src/lgbat_cmds.c
  fsw/src/lgbat_dispatch.c
)

# Create the app module
add_cfe_app(lgbat ${APP_SRC_FILES})

target_include_directories(lgbat PUBLIC
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

target_compile_options(lgbat PRIVATE -Wno-pedantic -Wno-error=pedantic)
target_compile_definitions(lgbat PRIVATE GS_PARAM_INTERNAL_USE=1)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    # OBC build
    target_link_libraries(lgbat ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
    # CPU1 build
    target_link_libraries(lgbat ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()
