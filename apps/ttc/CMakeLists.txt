project(CFE_TTC C)

set(APP_SRC_FILES
  fsw/src/ttc.c
  fsw/src/ttc_cmds.c
  fsw/src/ttc_utils.c
  fsw/src/ttc_timeline.c
)

if (CFE_EDS_ENABLED)
  list(APPEND APP_SRC_FILES
    fsw/src/ttc_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/ttc_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(ttc ${APP_SRC_FILES})

target_include_directories(ttc PUBLIC fsw/inc)
# to call library-provided functions
# Add table
# add_cfe_tables(ttc fsw/tables/ttc_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
