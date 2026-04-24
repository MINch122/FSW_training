##################################################################
#
# Coverage Unit Test build recipe
#
# This CMake file contains the recipe for building the RPT unit tests.
# It is invoked from the parent directory when unit tests are enabled.
#
##################################################################

#
# NOTE on the subdirectory structures here:
#
# - "inc" provides local header files shared between the coveragetest,
#    wrappers, and overrides source code units
# - "coveragetest" contains source code for the actual unit test cases
#    The primary objective is to get line/path coverage on the FSW
#    code units.
#

# Use the UT assert public API, and allow direct
# inclusion of source files that are normally private
# include_directories(../fsw/src)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_cfe_coverage_stubs("rpt_internal"
    stubs/rpt_cmd_stubs.c
    stubs/rpt_dispatch_stubs.c
    stubs/rpt_global_stubs.c
    stubs/rpt_init_stubs.c
    stubs/rpt_task_stubs.c
    stubs/rpt_utils_stubs.c
    utilities/rpt_test_utils.c
)

# Link with the cfe core stubs and unit test assert libs
target_link_libraries(coverage-rpt_internal-stubs ut_core_api_stubs ut_assert)

# Include and expose unit test utilities, fsw/inc, and fsw/src includes
# target_include_directories(coverage-rpt_internal-stubs PUBLIC coveragetest)
target_include_directories(coverage-rpt_internal-stubs PUBLIC utilities)
target_include_directories(coverage-rpt_internal-stubs PUBLIC ../fsw/inc)
target_include_directories(coverage-rpt_internal-stubs PUBLIC ../fsw/src)
target_include_directories(coverage-rpt_internal-stubs PUBLIC ../../../osal/src/ut-stubs)

# Stub includes needed for all targets
include_directories(stubs)

# Generate a dedicated "testrunner" executable for each test file
# Accomplish this by cycling through all the app's source files, there must be
# a *_tests file for each
foreach(SRCFILE ${APP_SRC_FILES})

    # Get the base sourcefile name as a module name without path or the
    # extension, this will be used as the base name of the unit test file.
    get_filename_component(UNIT_NAME "${SRCFILE}" NAME_WE)

    # Use the module name to make the test name by adding _tests to the end
    set(TESTS_NAME "coveragetest_${UNIT_NAME}")

    # Make the test sourcefile name with unit test path and extension
    set(TESTS_SOURCE_FILE "${PROJECT_SOURCE_DIR}/unit-test/${TESTS_NAME}.c")

    # Create the coverage test executable
    message(STATUS "CFE_RPT_SOURCE_DIR = ${CFE_RPT_SOURCE_DIR}/${SRCFILE}")
    add_cfe_coverage_test(rpt "${UNIT_NAME}" "${TESTS_SOURCE_FILE}" "${CFE_RPT_SOURCE_DIR}/${SRCFILE}")
    
    # Add dependency to utilities and internal stubs
    add_cfe_coverage_dependency(rpt "${UNIT_NAME}" rpt_internal)
    # target_link_libraries(coverage-rpt-${UNIT_NAME}-testrunner coverage-rpt-stubs)

endforeach()
