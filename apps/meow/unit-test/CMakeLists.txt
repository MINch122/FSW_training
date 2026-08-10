##################################################################
#
# MEOW unit test build recipe
#
# Tests are split into two groups:
#   core/  — pure POSIX modules (meow_shell, meow_file, meow_sys)
#   csp    — CSP/OSAL/FTP module (meow_csp) using hand-written stubs
#
# The cFS-layer modules (meow.c, meow_cmds.c, meow_dispatch.c) are
# not yet tested here; they require auto-generated message headers
# produced by the full EDS build.
#
##################################################################

include_directories(../fsw/src)
include_directories(../fsw/src/core)
include_directories(../fsw/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Stub library for the meow cFS-layer (not yet implemented; placeholder)
add_cfe_coverage_stubs(meow
  stubs/meow_stubs.c
  stubs/meow_global_stubs.c
  stubs/meow_cmds_stubs.c
  stubs/meow_dispatch_stubs.c
)

# -------------------------------------------------------------------------
# core/ tests — pure POSIX, no generated cFS headers needed
# -------------------------------------------------------------------------

foreach(CORE_UNIT meow_shell meow_file meow_sys)
    add_cfe_coverage_test(meow "${CORE_UNIT}"
        "${CMAKE_CURRENT_SOURCE_DIR}/coveragetest/coveragetest_${CORE_UNIT}.c"
        "../fsw/src/core/${CORE_UNIT}.c"
    )
endforeach()

if(MEOW_INCLUDE_CSP)

# -------------------------------------------------------------------------
# meow_csp — external deps stubbed in meow_csp_ut_stubs
# -------------------------------------------------------------------------

add_library(meow_csp_ut_stubs STATIC stubs/meow_csp_ext_stubs.c)
target_include_directories(meow_csp_ut_stubs PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    /usr/local/include
    ../fsw/src/core
)
target_link_libraries(meow_csp_ut_stubs osal_public_api)

add_cfe_coverage_test(meow "meow_csp"
    TESTCASE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/coveragetest/coveragetest_meow_csp.c"
    SUBJECT_SOURCES  "../fsw/src/core/meow_csp.c"
)

# The object under test needs CSP + GS FTP headers and the OSAL headers
# (OSAL is already on the path from the meow target; CSP/FTP need extras)
target_include_directories(coverage-meow-meow_csp-object PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    /usr/local/include
)

# The test driver additionally needs the core/ headers and stub declarations
target_include_directories(coverage-meow-meow_csp-testrunner PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    /usr/local/include
    ../fsw/src/core
)
target_link_libraries(coverage-meow-meow_csp-testrunner
    meow_csp_ut_stubs
    pthread
)

endif() # MEOW_INCLUDE_CSP
