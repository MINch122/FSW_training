project(CFE_PAYUZUT C)

set(APP_SRC_FILES
  fsw/src/payuzut_task.c
  fsw/src/payuzut_cmds.c
  fsw/src/payuzut_dispatch.c
  fsw/src/payuzut_utils.c
)

# Create the app module
add_cfe_app(payuzut ${APP_SRC_FILES})

target_include_directories(payuzut PUBLIC fsw/inc)