project(CFE_GPS_APP C)

set(APP_SRC_FILES
  fsw/src/gps_app.c
  fsw/src/gps_cmds.c
  fsw/src/gps_dispatch.c
  fsw/src/gps_report.c
  fsw/src/gps_service.c
  fsw/src/gps_cmds_oem.c
)

list(APPEND APP_SRC_FILES
  device/oem7/src/oem_cmd.c
  device/oem7/src/oem_log.c
  device/oem7/src/oem_task.c
  device/oem7/src/oem_io.c
  device/oem7/src/oem_utils.c
  device/oem7/callbacks/oem_cb.c
  device/oem7/callbacks/oem_cb_bestxyz.c
  device/oem7/callbacks/oem_cb_bestpos.c
  device/oem7/callbacks/oem_cb_range.c
  device/oem7/callbacks/oem_cb_time.c
  device/oem7/callbacks/oem_cb_clockmodel.c
  device/oem7/callbacks/oem_cb_rxstatus.c
  device/oem7/callbacks/oem_cb_satvis2.c
  device/oem7/callbacks/oem_cb_version.c
  device/oem7/callbacks/oem_cb_hwmonitor.c
)

# Append this only if the default linux serial callbacks are in use.
list(APPEND APP_SRC_FILES
  device/oem7/src/io_drivers/oem_io_serial_linux.c
)

add_cfe_app(gps ${APP_SRC_FILES})

target_include_directories(gps PUBLIC fsw/inc)
target_include_directories(gps PUBLIC fsw/src)
target_include_directories(gps PUBLIC device/oem7/inc)
target_include_directories(gps PUBLIC device/oem7/src)
target_include_directories(gps PUBLIC device/oem7/callbacks)
