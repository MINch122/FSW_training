project(CFE_PAYUEL_ROMA C)

set(APP_SRC_FILES
  fsw/src/payuel_roma.c
  fsw/src/payuel_roma_cmds.c
  fsw/src/payuel_roma_utils.c
  device/src/s5lab.c
  device/src/s5lab_csp.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/payuel_roma_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/payuel_roma_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(payuel_roma ${APP_SRC_FILES})
target_compile_options(payuel_roma PRIVATE -Wno-pedantic)
target_include_directories(payuel_roma PUBLIC fsw/inc device/inc device/test)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions


# Add table
add_cfe_tables(payuel_roma fsw/tables/payuel_roma_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
