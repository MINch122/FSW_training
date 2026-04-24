#!/bin/bash

# pyenv 설정
export PATH="$HOME/.pyenv/bin:$PATH"
eval "$(pyenv init --path)"
eval "$(pyenv init -)"
export PYENV_VERSION=3.10.13

cpu1_install_path=$(pwd)/submodules/libgscsp/build/cpu1
obc_install_path=$(pwd)/submodules/libgscsp/build/obc

toolchain_bin_path=$(pwd)/submodules/toolchain/bin
toolchain_sysroot=$(pwd)/submodules/toolchain/arm-buildroot-linux-gnueabi/sysroot

cd submodules/libgscsp

python3 ./tools/buildtools/gsbuildtools_bootstrap.py

python3 waf distclean

# cpu1 build (x86 host — libsocketcan not available; wscript auto-detects)
CFLAGS='-fPIC' python3 waf configure --prefix=$cpu1_install_path --libdir=$cpu1_install_path/lib
python3 waf build install

# obc build (ARM cross-compile — libsocketcan in toolchain sysroot)
export PKG_CONFIG_PATH=$toolchain_sysroot/usr/lib/pkgconfig:$PKG_CONFIG_PATH
CFLAGS='-fPIC' python3 waf configure --prefix=$obc_install_path --libdir=$obc_install_path/lib --toolchain=$toolchain_bin_path/arm-buildroot-linux-gnueabi- --arch=arm
python3 waf build install
