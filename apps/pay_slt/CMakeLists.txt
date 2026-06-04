project(CFE_SLT_IFB C)

set(APP_SRC_FILES
  fsw/src/SLT_IFB_app.c
  fsw/src/SLT_IFB_cmds.c
  fsw/src/SLT_IFB_utils.c
  device/src/SLT_IFB.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/SLT_IFB_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/SLT_IFB_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(pay_slt ${APP_SRC_FILES})

set_target_properties(pay_slt PROPERTIES OUTPUT_NAME "SLT_IFB")

target_include_directories(pay_slt PUBLIC 
fsw/inc 
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

target_compile_options(pay_slt PRIVATE -Wno-pedantic -Wno-error=pedantic)
