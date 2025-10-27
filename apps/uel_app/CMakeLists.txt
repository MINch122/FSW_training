project(CFE_UEL_APP C)

set(APP_SRC_FILES
  fsw/src/uel_app.c
  fsw/src/uel_app_cmds.c
  fsw/src/uel_app_dispatch.c
  fsw/src/uel_app_utils.c
)

# Create the app module
add_cfe_app(uel_app ${APP_SRC_FILES})

target_include_directories(uel_app PUBLIC fsw/inc)
# Include the public API from uel_lib to demonstrate how

# Link CSP client & util libraries
# target_include_directories(uel_app PRIVATE
#   ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include/
#   ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/lib/libutil/include/
#   ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include/
# )

# target_compile_options(uel_app PRIVATE -Wno-pedantic -Wno-error=pedantic)

# -----------------------------------------------------------------
# Conditional linking depending on target processor
# -----------------------------------------------------------------
# message(STATUS "Processor = ${CMAKE_SYSTEM_PROCESSOR}")

# if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
#     # OBC build
#     target_link_libraries(uel_app ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
# else()
#     # Host simulation build
#     target_link_libraries(uel_app ${PROJECT_SOURCE_DIR}/../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
# endif()
# to call library-provided functions

#add_cfe_app_dependency(uel_app sample_lib)

# Add table
# add_cfe_tables(uel_app fsw/tables/uel_app_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
# if (ENABLE_UNIT_TESTS)
#   add_subdirectory(unit-test)
# endif (ENABLE_UNIT_TESTS)

# target_compile_options(uel_app PRIVATE -Wno-pedantic -Wno-error=pedantic)
