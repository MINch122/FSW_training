add_compile_options(-std=gnu11 -Wno-error)
project(CFE_UANT_APP C)

set(APP_SRC_FILES
  fsw/src/uant_app.c
  fsw/src/uant_cmds.c
  fsw/src/uant_dispatch.c
  device/src/uant.c
)

# Create the app module
add_cfe_app(uant ${APP_SRC_FILES})

#target_include_directories(uant_app PUBLIC fsw/inc)
target_include_directories(uant PUBLIC
    fsw/inc
    fsw/src                    # 앱 내부 헤더
    ${CMAKE_CURRENT_SOURCE_DIR}/device/inc/uant   # uant.h가 있는 경로
)
target_include_directories(uant PUBLIC /usr/include)
# Include the public API from uant_lib to demonstrate how
# to call library-provided functions
#add_cfe_app_dependency(uant_app)

# Add table
# add_cfe_tables(uant_app fsw/tables/uant_app_tbl.c)



