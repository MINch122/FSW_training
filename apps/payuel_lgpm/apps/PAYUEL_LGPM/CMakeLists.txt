project(CFE_PAYUEL_LGPM_APP C)

set(APP_SRC_FILES
  fsw/src/PAYUEL_LGPM_app.c
  fsw/src/PAYUEL_LGPM_cmds.c
  fsw/src/PAYUEL_LGPM_utils.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/PAYUEL_LGPM_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/PAYUEL_LGPM_dispatch.c
  )
endif()

# Create the app module 
add_cfe_app(PAYUEL_LGPM ${APP_SRC_FILES})

target_include_directories(PAYUEL_LGPM PUBLIC fsw/inc)

# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
add_cfe_app_dependency(PAYUEL_LGPM sample_lib)

# Add table
add_cfe_tables(PAYUEL_LGPM fsw/tables/PAYUEL_LGPM_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
