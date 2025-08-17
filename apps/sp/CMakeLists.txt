project(CFE_SP C)

set(APP_SRC_FILES
  fsw/src/sp_task.c
  fsw/src/sp_cmds.c
  fsw/src/sp_dispatch.c
  fsw/src/sp_utils.c
)

# Create the app module
add_cfe_app(sp ${APP_SRC_FILES})

target_include_directories(sp PUBLIC fsw/inc)

# Include the public API from sp_lib to demonstrate how
# to call library-provided functions
#add_cfe_app_dependency(sp sp_lib)

# Add table
# add_cfe_tables(sp fsw/tables/sp_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
#if (ENABLE_UNIT_TESTS)
#  add_subdirectory(unit-test)
#endif (ENABLE_UNIT_TESTS)