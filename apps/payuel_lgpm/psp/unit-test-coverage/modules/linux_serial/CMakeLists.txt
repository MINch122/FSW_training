######################################################################
#
# CMAKE build recipe for white-box coverage tests of VxWorks timebase module
#
add_definitions(-D_CFE_PSP_MODULE_)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc")
include_directories("${CFEPSP_SOURCE_DIR}/fsw/modules/iodriver/inc")
# include_directories("${CFEPSP_SOURCE_DIR}/fsw/modules/linux_serial")

add_psp_covtest(linux_serial coveragetest-linux_serial.c
    ${CFEPSP_SOURCE_DIR}/fsw/modules/linux_serial/linux_serial.c
    ${CFEPSP_SOURCE_DIR}/fsw/modules/iodriver/src/iodriver.c
)
