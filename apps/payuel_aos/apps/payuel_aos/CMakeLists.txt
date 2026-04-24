project(CFE_PAYUEL_AOS C)

set(APP_SRC_FILES
  fsw/src/payuel_aos.c
  fsw/src/payuel_aos_cmds.c
  fsw/src/payuel_aos_utils.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/payuel_aos_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/payuel_aos_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(payuel_aos ${APP_SRC_FILES})

target_include_directories(payuel_aos PUBLIC fsw/inc)
# to call library-provided functions
# Add table
add_cfe_tables(payuel_aos fsw/tables/payuel_aos_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
