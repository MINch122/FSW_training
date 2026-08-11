project(CFE_THRUST_APP C)

include_directories(device/inc)
include_directories(fsw/inc)
# include_directories(../../cfe/modules/srl/fsw/src)

set(SRC_APP_FILES
  fsw/src/thrust_app.c
  fsw/src/thrust_cmds.c
  fsw/src/thrust_dispatch.c
)

set(SRC_DEV_FILES
  device/src/thrust_cmds_list.c
  device/src/thrust_crc.c
)

# # Create the app module
add_cfe_app(thrust ${SRC_APP_FILES} ${SRC_DEV_FILES} ${SRC_EXTRA_FILES})


target_include_directories(thrust PUBLIC fsw/inc)
target_include_directories(thrust PUBLIC device/inc)
# target_include_directories(thrust PUBLIC ../../cfe/modules/srl/config)
