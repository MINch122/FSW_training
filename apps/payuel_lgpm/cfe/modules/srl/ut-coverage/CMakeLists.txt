##################################################################
#
# cFE SRL unit test CMake build recipe
#
##################################################################
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pedantic -Wno-error=pedantic")

foreach(SRC ${srl_SOURCES})
    list(APPEND UT_SOURCES "${CFE_SRL_SOURCE_DIR}/${SRC}")
endforeach()

add_cfe_coverage_test(srl ALL
    "srl_UT.c"
    "${UT_SOURCES}"
)
# Prevent csp error
# target_compile_options(coverage-srl-ALL-testrunner PRIVATE -Wno-pedantic -Wno-error=pedantic)
# target_compile_options(coverage-srl-ALL PRIVATE -Wno-pedantic -Wno-error=pedantic)
# Link CSP
include_directories(../../../submodules/libgscsp/lib/libgattos-uc_client_client/include)
include_directories(../../../submodules/libgscsp/lib/isl-modem-app_client/include)
include_directories(../../../submodules/libgscsp/lib/libaardvark/include)
include_directories(../../../submodules/libgscsp/lib/libadcs_client/include)
include_directories(../../../submodules/libgscsp/lib/libfp_client/include)
include_directories(../../../submodules/libgscsp/lib/libftp_client/include)
include_directories(../../../submodules/libgscsp/lib/libgosh_client/include)
include_directories(../../../submodules/libgscsp/lib/libgscsp/include)
include_directories(../../../submodules/libgscsp/lib/libgscsp/lib/libcsp/include)
include_directories(../../../submodules/libgscsp/lib/libgscsp/lib/libcsp/src) # #include <csp_conn.h>
include_directories(../../../submodules/libgscsp/lib/libgssb_client/include)
include_directories(../../../submodules/libgscsp/lib/libhk_client/include)
include_directories(../../../submodules/libgscsp/lib/libnanopb_client/include)
include_directories(../../../submodules/libgscsp/lib/libp60_client/include)
include_directories(../../../submodules/libgscsp/lib/libparam_client/include)
include_directories(../../../submodules/libgscsp/lib/librgosh_client/include)
include_directories(../../../submodules/libgscsp/lib/libutil/include)
include_directories(../../../submodules/libgscsp/build/lib/libgscsp/lib/libcsp/include)
if (CMAKE_SYSTEM_PROCESSOR	STREQUAL "arm")
    # For OBC
    # link csp
    target_link_libraries(coverage-srl-ALL-testrunner ${PROJECT_SOURCE_DIR}/../../../submodules/libgscsp/build/obc/lib/libcsp-client.a)
else ()
    # For local Linux host
    # link csp
    target_link_libraries(coverage-srl-ALL-testrunner ${PROJECT_SOURCE_DIR}/../../../submodules/libgscsp/build/cpu1/lib/libcsp-client.a)
endif()

# This permits UT test cases to directly access private headers in the fsw/src dir
target_include_directories(coverage-srl-ALL-testrunner PRIVATE
    ${CFE_SRL_SOURCE_DIR}/fsw/src
)

target_link_libraries(coverage-srl-ALL-testrunner ut_core_private_stubs)