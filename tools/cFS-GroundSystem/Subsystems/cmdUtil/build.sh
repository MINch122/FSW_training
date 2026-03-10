#!/bin/sh
# cmdUtil 빌드 스크립트 - CMake 없이 직접 컴파일
cd "$(dirname "$0")"
gcc -o cmdUtil cmdUtil.c SendUdp.c -Wall
echo "Build complete: $(pwd)/cmdUtil"
