project(CFE_PAYUZUC C)

set(APP_SRC_FILES
  fsw/src/payuzuc_task.c
  fsw/src/payuzuc_cmds.c
  fsw/src/payuzuc_dispatch.c
  fsw/src/payuzuc_utils.c
)

# Create the app module
add_cfe_app(payuzuc ${APP_SRC_FILES})

target_include_directories(payuzuc PUBLIC fsw/inc)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
# add_cfe_app_dependency(payuzuc sample_lib)

# Add table
# add_cfe_tables(payuzuc fsw/tables/payuzuc_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
# if (ENABLE_UNIT_TESTS)
#   add_subdirectory(unit-test)
# endif (ENABLE_UNIT_TESTS)
