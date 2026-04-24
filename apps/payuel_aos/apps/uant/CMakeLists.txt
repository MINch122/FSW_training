##################################################################
#
# uANT Application (uant_app) CMake build recipe
#
# Author : Original Author
##################################################################

project (uant_app C)

# -----------------------------------------------------------------
# Source Files
# -----------------------------------------------------------------
set(APP_SRC_FILES
    fsw/src/uant_app.c
    fsw/src/uant_app_cmds.c
    fsw/src/uant_app_utils.c
    fsw/src/uant_app_dispatch.c
)

# -----------------------------------------------------------------
# Create the cFE application module
# -----------------------------------------------------------------
add_cfe_app(uant_app ${APP_SRC_FILES})

target_include_directories(uant_app PUBLIC fsw/inc)

# Link CSP client & util libraries
target_include_directories(uant_app PRIVATE
  ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/lib/libgssb_client/include
  ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/lib/libutil/include
)

target_compile_options(uant_app PRIVATE -Wno-pedantic -Wno-error=pedantic)

# -----------------------------------------------------------------
# Conditional linking depending on target processor
# -----------------------------------------------------------------
message(STATUS "Processor = ${CMAKE_SYSTEM_PROCESSOR}")

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
    # OBC build
    target_link_libraries(uant_app ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else()
    # Host simulation build
    target_link_libraries(uant_app ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()
