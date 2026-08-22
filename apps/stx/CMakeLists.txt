project(CFE_STX C)

set(SRC_APP_FILES
  fsw/src/stx.c
  fsw/src/stx_cmds.c
  fsw/src/stx_utils.c
  fsw/src/stx_dispatch.c
)

set(STX_DEV_SRC
  device/src/enduro_stx.c
  device/src/esup_framer.c
  device/src/esup_session.c
  device/src/esup_utils.c
  device/src/esup_wire_linux.c
  device/src/esup.c
)

# Create the app module
add_cfe_app(stx ${STX_DEV_SRC} ${SRC_APP_FILES})

target_include_directories(stx PRIVATE device/inc fsw/src PUBLIC fsw/inc)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
# add_cfe_app_dependency(stx sample_lib)

# Add table
# add_cfe_tables(stx fsw/tables/stx_tbl.c)
