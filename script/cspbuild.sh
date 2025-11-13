#!/bin/bash

cpu1_install_path=$(pwd)/submodules/libgscsp/build/cpu1
obc_install_path=$(pwd)/submodules/libgscsp/build/obc
pi_install_path=$(pwd)/submodules/libgscsp/build/pi

toolchain_bin_path=$(pwd)/submodules/toolchain/bin
toolchain_lib_path=$(pwd)/submodules/toolchain/lib

cd submodules/libgscsp

python3 ./tools/buildtools/gsbuildtools_bootstrap.py

python3 waf distclean

# cpu1 build
CFLAGS='-fPIC' python3 waf configure --prefix=$cpu1_install_path
python3 waf build install

# obc build
export PATH=bin_path:$PATH
#export LD_LIBRARY_PATH=$lib_path:$LD_LIBRARY_PATH
CFLAGS='-fPIC' python3 waf configure --prefix=$obc_install_path --toolchain=$toolchain_bin_path/arm-buildroot-linux-gnueabi- --arch=arm
python3 waf build install

# pi build
CFLAGS='-fPIC' python3 waf configure --prefix=$pi_install_path --part=aarch64-linux
python3 waf build install