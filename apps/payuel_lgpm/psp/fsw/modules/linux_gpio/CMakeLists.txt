# Pseudo-terminal interface module
add_psp_module(linux_gpio linux_gpio.c)
target_include_directories(linux_gpio PRIVATE $<TARGET_PROPERTY:iodriver,INTERFACE_INCLUDE_DIRECTORIES>)


message(STATUS "Processor = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")


message(STATUS "GPIOD LIB: ${PROJECT_SOURCE_DIR}/../../../submodules/libgpiod/obc/lib/libgpiod.a")
message(STATUS "GPIOD INCLUDE: ${PROJECT_SOURCE_DIR}/../../../submodules/libgpiod/obc/include")

if (CMAKE_SYSTEM_PROCESSOR	STREQUAL "arm")
    # For OBC (cross-compiled libgpiod)
    # gpiod
    target_include_directories(linux_gpio PRIVATE ${PROJECT_SOURCE_DIR}/../submodules/libgpiod/obc/include)
    target_link_libraries(linux_gpio PRIVATE ${PROJECT_SOURCE_DIR}/../submodules/libgpiod/obc/lib/libgpiod.a)
else ()
    # For local Linux host
    # link libgpiod.so -> drop this
    #find_library(GPIOD_LIB gpiod REQUIRED)
    # Must enter the keyword `PRIVATE`
    #target_link_libraries(linux_gpio PRIVATE ${GPIOD_LIB})

    # link libgpiod.a
    target_include_directories(linux_gpio PRIVATE /usr/include)
    target_link_libraries(linux_gpio PRIVATE /usr/lib/x86_64-linux-gnu/libgpiod.a)
endif()