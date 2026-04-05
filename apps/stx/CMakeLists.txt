project(CFE_STX C)

include_directories(device/inc)
include_directories(fsw/inc)
include_directories(../../cfe/modules/srl/fsw/src)



set(SRC_APP_FILES
  fsw/src/stx.c
  fsw/src/stx_cmds.c
  fsw/src/stx_utils.c
  fsw/src/stx_dispatch.c
)

# stx_app.c는 제외하고 나머지만 수동 등록
set(STX_DEV_SRC
  device/src/crc32.c
  device/src/esup.c
  # device/src/rs232.c
  # device/src/rs485.c
)

set(STX_EXTRA_SRC
  ../../cfe/modules/srl/fsw/src/cfe_srl_api.c
)


# Create the app module
add_cfe_app(stx ${STX_DEV_SRC} ${SRC_APP_FILES})

target_include_directories(stx PUBLIC fsw/inc)
target_include_directories(stx PUBLIC device/inc)
target_include_directories(stx PUBLIC ../../cfe/modules/srl/config)
target_include_directories(stx PUBLIC /usr/include)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
# add_cfe_app_dependency(stx sample_lib)

# Add table
# add_cfe_tables(stx fsw/tables/stx_tbl.c)
