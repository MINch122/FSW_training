project(CFE_EPS_APP C)


set(APP_SRC_FILES
  fsw/src/eps_app.c
  fsw/src/eps_cmds.c
  fsw/src/eps_utils.c
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


target_include_directories(eps PUBLIC
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

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
