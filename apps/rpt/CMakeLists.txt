project (CFE_RPT C)

set (APP_SRC_FILES
    fsw/src/rpt_cmd.c
    fsw/src/rpt_dispatch.c
    fsw/src/rpt_init.c
    fsw/src/rpt_task.c
    fsw/src/rpt_utils.c)
    
# Create the app module
add_cfe_app(rpt ${APP_SRC_FILES})

# The API to this library (which may be invoked/referenced from other apps)
# is stored in fsw/public_inc.  Using "target_include_directories" is the 
# preferred method of indicating this (vs. directory-scope "include_directories").
target_include_directories(rpt PUBLIC fsw/inc)

# Add table
add_cfe_tables(rpt fsw/tables/rpt_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)