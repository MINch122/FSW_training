# Pseudo-terminal interface module
add_psp_module(linux_serial linux_serial.c)
target_include_directories(linux_serial PRIVATE $<TARGET_PROPERTY:iodriver,INTERFACE_INCLUDE_DIRECTORIES>)