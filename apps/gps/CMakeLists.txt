project(CFE_GPS_APP C)

set(APP_SRC_FILES
  fsw/src/gps_app.c
  fsw/src/gps_cmds.c
  fsw/src/gps_cmds_oem.c
  fsw/src/gps_dev_oem.c
)

list(APPEND APP_SRC_FILES
  device/oem7/src/oem_cmd.c
  device/oem7/src/oem_log.c
  device/oem7/src/oem_task.c
  device/oem7/src/oem_io.c
  device/oem7/src/oem_utils.c
  device/oem7/callbacks/oem_log_cb.c
)

# Append this only if the default linux serial callbacks are in use.
list(APPEND APP_SRC_FILES
  device/oem7/src/arch/oem_arch_io_linux.c
)

list(APPEND APP_SRC_FILES
  device/oem7/mlk/list/src/mlk_list.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/gps_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/gps_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(gps ${APP_SRC_FILES})

target_include_directories(gps PUBLIC fsw/inc)
target_include_directories(gps PUBLIC device/oem7/inc)
target_include_directories(gps PUBLIC device/oem7/src)
target_include_directories(gps PUBLIC device/oem7/mlk)
target_include_directories(gps PUBLIC device/oem7/mlk/list/inc)
target_include_directories(gps PUBLIC device/oem7/mlk/list/src)
target_include_directories(gps PUBLIC device/oem7/callbacks)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
# if (ENABLE_UNIT_TESTS)
#   add_subdirectory(unit-test)
# endif (ENABLE_UNIT_TESTS)
