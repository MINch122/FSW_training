FROM ubuntu:22.04
RUN apt-get update && apt-get install -y gcc && rm -rf /var/lib/apt/lists/*
WORKDIR /test
# Build context is the repo root (apps/meow/)
COPY fsw/src/core/meow_sys.c \
     fsw/src/core/meow_sys.h \
     fsw/src/core/meow_shell.c \
     fsw/src/core/meow_shell.h \
     unit-test/danger/test_danger.c \
     ./
RUN gcc -Wall -Wextra -o test_danger test_danger.c meow_sys.c meow_shell.c
