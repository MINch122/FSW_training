# This example toolchain file describes the cross compiler to use for
# the target architecture indicated in the configuration file.

# Basic cross system configuration
SET(CMAKE_SYSTEM_NAME			Linux)
SET(CMAKE_SYSTEM_VERSION		1)
SET(CMAKE_SYSTEM_PROCESSOR		aarch64)

# adjust these settings to where the cross compiler actually resides
SET(CMAKE_C_COMPILER			"/usr/bin/aarch64-linux-gnu-gcc")
SET(CMAKE_CXX_COMPILER			"/usr/bin/aarch64-linux-gnu-g++")

# where is the target environment
# Don't use any custom sysroot
# SET(CMAKE_FIND_ROOT_PATH		"${CMAKE_CURRENT_SOURCE_DIR}/../submodules/toolchain/arm-buildroot-linux-gnueabi/sysroot")

# Configure the find commands
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM	NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY	NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE	NEVER)

# these settings are specific to cFE/OSAL and determines which 
# abstraction layers are built when using this toolchain
# Note that "pc-linux" works fine even though this is not technically a "pc"
SET(CFE_SYSTEM_PSPNAME      "pc-linux")
SET(OSAL_SYSTEM_OSTYPE      "posix")
