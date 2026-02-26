project(CFE_SP C)

set(APP_SRC_FILES
  fsw/src/sp_task.c
  fsw/src/sp_cmds.c
  fsw/src/sp_dispatch.c
  fsw/src/sp_utils.c
)

# Create the app module
add_cfe_app(sp ${APP_SRC_FILES})

target_include_directories(sp PUBLIC
  fsw/inc
  ../../submodules/libgscsp/lib/libgssb_client/include
  ../../submodules/libgscsp/lib/libutil/include
)

target_compile_options(sp PRIVATE -Wno-pedantic -Wno-error=pedantic)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    # OBC build
    target_link_libraries(sp ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
    # CPU1 build
    target_link_libraries(sp ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()