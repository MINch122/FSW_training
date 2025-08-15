project(CFE_STRX_APP C)

set(APP_SRC_FILES
  fsw/src/strx_app.c
  fsw/src/strx_app_cmds.c
  fsw/src/strx_app_utils.c
  fsw/src/strx_app_dispatch.c
  device/src/strx.c
)

# # Create the app module
add_cfe_app(strx ${APP_SRC_FILES})

target_include_directories(strx PUBLIC
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

target_compile_options(strx PRIVATE -Wno-pedantic -Wno-error=pedantic)