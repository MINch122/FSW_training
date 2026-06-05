project(CFE_LTRX_APP C)

set(APP_SRC_FILES
  fsw/src/ltrx_app.c
  fsw/src/ltrx_child.c
  fsw/src/ltrx_cmds_beacon.c
  fsw/src/ltrx_cmds.c
  fsw/src/ltrx_dispatch.c
  fsw/src/ltrx_utils.c
  fsw/src/ltrx_session.c
  device/src/ltrx.c
)

# # Create the app module
add_cfe_app(ltrx ${APP_SRC_FILES})

target_include_directories(ltrx PUBLIC
  fsw/inc
  fsw/src
  device/inc
../../submodules/libgscsp/lib/libgattos-uc_client_client/include
../../submodules/libgscsp/lib/isl-modem-app_client/include
../../submodules/libgscsp/lib/libaardvark/include
../../submodules/libgscsp/lib/libadcs_client/include
../../submodules/libgscsp/lib/libfp_client/include
../../submodules/libgscsp/lib/libftp_client/include
../../submodules/libgscsp/lib/libgosh_client/include
../../submodules/libgscsp/lib/libgscsp/include
../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include
../../submodules/libgscsp/lib/libgscsp/lib/libcsp/src # #include <csp_conn.h>
../../submodules/libgscsp/lib/libgssb_client/include
../../submodules/libgscsp/lib/libhk_client/include
../../submodules/libgscsp/lib/libnanopb_client/include
../../submodules/libgscsp/lib/libp60_client/include
../../submodules/libgscsp/lib/libparam_client/include
../../submodules/libgscsp/lib/libparam_client/include/deprecated
../../submodules/libgscsp/lib/librgosh_client/include
../../submodules/libgscsp/lib/libutil/include
../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include
../../submodules/libgscsp/lib/nanocom-ax_client/include
)

target_compile_options(ltrx PRIVATE -Wno-pedantic -Wno-error=pedantic)