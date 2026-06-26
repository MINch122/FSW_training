project(CFE_paybee_kisscam C)

set(APP_SRC_FILES
  fsw/src/paybee_kisscam_task.c
  fsw/src/paybee_kisscam_cmds.c
  fsw/src/paybee_kisscam_dispatch.c
  fsw/src/paybee_kisscam_utils.c
)

# Create the app module
add_cfe_app(paybee_kisscam ${APP_SRC_FILES})

target_include_directories(paybee_kisscam PUBLIC fsw/inc)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
# add_cfe_app_dependency(paybee_kisscam sample_lib)

# Add table
# add_cfe_tables(paybee_kisscam fsw/tables/paybee_kisscam_tbl.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
# if (ENABLE_UNIT_TESTS)
#   add_subdirectory(unit-test)
# endif (ENABLE_UNIT_TESTS)


# ==================================================
# libaec 라이브러리 다운로드 및 빌드 통합
# ==================================================

include(FetchContent)

FetchContent_Declare(
  libaec
  GIT_REPOSITORY https://github.com/Deutsches-Klimarechenzentrum/libaec.git
  GIT_TAG        v1.1.7
)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(libaec)

if(TARGET aec-static-objects)
  target_compile_options(aec-static-objects PRIVATE -Wno-error=strict-prototypes -Wno-strict-prototypes)
endif()
if(TARGET sz-static-objects)
  target_compile_options(sz-static-objects PRIVATE -Wno-error=strict-prototypes -Wno-strict-prototypes)
endif()
if(TARGET aec-static)
  target_compile_options(aec-static PRIVATE -Wno-error=strict-prototypes -Wno-strict-prototypes)
endif()

target_link_libraries(paybee_kisscam aec-static)