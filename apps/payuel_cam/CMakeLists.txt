project(CFE_PAYUEL_CAM C)

set(APP_SRC_FILES
  fsw/src/payuel_cam.c
  fsw/src/payuel_cam_child.c
  fsw/src/payuel_cam_cmds.c
  fsw/src/payuel_cam_utils.c
  fsw/src/payuel_cam_dispatch.c
)

add_cfe_app(payuel_cam ${APP_SRC_FILES})

target_include_directories(payuel_cam PUBLIC fsw/inc)

target_include_directories(payuel_cam PRIVATE
    ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/lib/libgscsp/include
    ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/lib/libgscsp/include/gs/csp
    ${PROJECT_SOURCE_DIR}/../../cfe/modules/srl/fsw/src
)

target_compile_options(payuel_cam PRIVATE -Wno-pedantic -Wno-error=pedantic)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    target_link_libraries(payuel_cam ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
    target_link_libraries(payuel_cam ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()

add_cfe_tables(payuel_cam fsw/tables/payuel_cam_tbl.c)
