project(CFE_paybee_kisscam C)

set(APP_SRC_FILES
  fsw/src/paybee_kisscam_task.c
  fsw/src/paybee_kisscam_cmds.c
  fsw/src/paybee_kisscam_dispatch.c
  fsw/src/paybee_kisscam_utils.c
)

# Create the app module
add_cfe_app(paybee_kisscam ${APP_SRC_FILES})

target_include_directories(paybee_kisscam PUBLIC fsw/inc)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
# add_cfe_app_dependency(paybee_kisscam sample_lib)

# Add table
# add_cfe_tables(paybee_kisscam fsw/tables/paybee_kisscam_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
# if (ENABLE_UNIT_TESTS)
#   add_subdirectory(unit-test)
# endif (ENABLE_UNIT_TESTS)
