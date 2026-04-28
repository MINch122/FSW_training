project(CFE_PAYUEL_LGPM_APP C)

set(APP_SRC_FILES
  fsw/src/payuel_lgpm_app.c
  fsw/src/payuel_lgpm_cmds.c
  fsw/src/payuel_lgpm_utils.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/payuel_lgpm_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/payuel_lgpm_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(payuel_lgpm ${APP_SRC_FILES})

target_include_directories(payuel_lgpm PUBLIC fsw/inc)

# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
add_cfe_app_dependency(payuel_lgpm sample_lib)

# Add table
add_cfe_tables(payuel_lgpm fsw/tables/payuel_lgpm_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
