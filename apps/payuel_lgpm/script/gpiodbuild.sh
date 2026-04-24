#!/bin/bash

# Build libgpiod via OBC toolchain
toolchain_bin_path=$(pwd)/submodules/toolchain/bin
export PATH=$toolchain_bin_path:$PATH

cd submodules/libgpiod
make distclean

./autogen.sh\
 --host=arm-buildroot-linux-gnueabi\
 --prefix=$(pwd)/obc\
 --disable-tools

make -j$(nproc)
make install