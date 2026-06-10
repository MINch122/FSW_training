project(CFE_EPS_APP C)


set(APP_SRC_FILES
  fsw/src/eps_app.c
  fsw/src/eps_cmds.c
  fsw/src/eps_utils.c
  device/p80/eps_p80_drv.c
  device/bp8/eps_bp8_drv.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/eps_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/eps_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(eps ${APP_SRC_FILES})

target_include_directories(eps PUBLIC fsw/inc)
target_include_directories(eps PUBLIC device/p80 device/bp8)


target_include_directories(eps PUBLIC
../../cfe/modules/srl/fsw/src
../../submodules/libgscsp/lib/libftp_client/include
../../submodules/libgscsp/lib/libgscsp/include
../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include
../../submodules/libgscsp/lib/libhk_client/include
../../submodules/libgscsp/lib/libparam_client/include
../../submodules/libgscsp/lib/libparam_client/include/deprecated
../../submodules/libgscsp/lib/libutil/include
../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include
../../submodules/libgscsp/lib/libp80_client/include
../../submodules/libgscsp/lib/p80-pdu_client/include
../../submodules/libgscsp/lib/p80-pmu_client/include
../../submodules/libgscsp/lib/p80-acu_client/include
)

target_compile_options(eps PRIVATE -Wno-pedantic -Wno-error=pedantic)
target_compile_definitions(eps PRIVATE GS_PARAM_INTERNAL_USE=1)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    # OBC build
    target_link_libraries(eps ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    # For PI
    target_link_libraries(eps ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/pi/lib/libcsp-client.a)
else()
    # CPU1 build
    target_link_libraries(eps ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS AND NOT SCH_LAB_UNIT_TEST_ONLY)
  add_subdirectory(unit-test)
endif ()
