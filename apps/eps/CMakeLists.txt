project(CFE_EPS_APP C)

set(APP_SRC_FILES
  fsw/src/eps_app.c
  fsw/src/eps_cmds.c
  fsw/src/eps_cmds_p31u.c
)

list(APPEND APP_SRC_FILES
  device/p31u/src/p31u.c
  device/p31u/src/ifc/p31u_i2c_srl.c
)

# Create the app module
add_cfe_app(eps ${APP_SRC_FILES})

target_include_directories(eps PUBLIC fsw/inc)
target_include_directories(eps PUBLIC fsw/src)
target_include_directories(eps PUBLIC device/p31u/inc)
