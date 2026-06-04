# Build the ftpnew test_server / test_client binaries.
#
# Usage:
#   make                        # native, one-shot compile+link, no .o files
#   make TARGET=arm32           # cross-build for ARM32
#   make KEEP_OBJ=true          # also emit per-source .o under build/<target>/
#   make install                # copy runtime libs into build/<target>/lib/
#                               # so the build tree is self-contained
#
# Per-target outputs land in build/$(TARGET)/. The binaries embed an rpath
# of '$ORIGIN/lib:$ORIGIN/../../external/lib/$(TARGET)' (DT_RPATH, not
# DT_RUNPATH), which covers both pre-install (run from the source tree)
# and post-install (run from a bundled / shipped tree).
#
# No object files are written anywhere except under build/, and only when
# KEEP_OBJ=true.

TARGET   ?= native
KEEP_OBJ ?= false

# Use ':=' (not '?=') because make pre-defines CC = cc as a built-in
# default, which would defeat '?='. A command-line 'make CC=...' override
# still wins because GNU make ignores in-Makefile assignments to variables
# set on the command line.
ifeq ($(TARGET),native)
    CC := gcc
else ifeq ($(TARGET),arm32)
    CC := arm-linux-gnueabi-gcc
else
    $(error TARGET must be 'native' or 'arm32' (got '$(TARGET)'))
endif

EXT_DIR     := external
EXT_INC     := $(EXT_DIR)/include
EXT_LIB_DIR := $(EXT_DIR)/lib/$(TARGET)

SERVER_SRC := ../server/src
SERVER_INC := ../server/inc
CONFIG_DIR := ../config
CLIENT_SRC := ../client/src
CLIENT_INC := ../client/inc
BUILDDIR   := build/$(TARGET)

# Where 'make install' bundles the runtime libs.
INSTALL_LIB_DIR := $(BUILDDIR)/lib

CFLAGS  := -std=gnu11 -O2 -g -Wall -Wextra -Wno-unused-parameter \
           -I. \
           -I$(SERVER_INC) \
           -I$(CLIENT_INC) \
           -I$(CONFIG_DIR) \
           -I$(EXT_INC)

# rpath is relative to $ORIGIN (the directory of the executable). We list
# two paths so the binary works in both layouts:
#   1) '$ORIGIN/lib'                 -> populated by 'make install',
#                                       used when the build tree is
#                                       shipped as a self-contained bundle.
#   2) '$ORIGIN/../../<external>'    -> in-tree dev fallback when 'install'
#                                       hasn't been run yet.
#
# '--disable-new-dtags' forces DT_RPATH (not DT_RUNPATH). DT_RPATH applies
# to the binary's direct deps AND every transitive load they trigger; DT_RUNPATH
# only applies to direct deps and lets transitive loads fail with the
# misleading "cannot open shared object" error.
LDFLAGS := -L$(EXT_LIB_DIR) \
           -Wl,-rpath,'$$ORIGIN/lib:$$ORIGIN/../../$(EXT_LIB_DIR)' \
           -Wl,--disable-new-dtags

LDLIBS  := -lcsp -lgsutil -lpthread -lrt

# Runtime libs to bundle under $(INSTALL_LIB_DIR) when 'make install' runs.
# Copies ALL .so files from $(EXT_LIB_DIR) -- the binaries only directly
# link a subset, but transitive deps are routinely pulled in via gscsp /
# gsftp_client, so bundle everything to keep the deploy unit honest.
INSTALL_LIBS := libcsp.so libgscsp.so libgsftp_client.so libgsutil.so

# Per-binary source lists. test_common.c + ftpnew_utils.c are shared.
COMMON_SRCS := test_common.c \
               $(SERVER_SRC)/ftpnew_utils.c

SERVER_SRCS := test_server.c \
               $(COMMON_SRCS) \
               $(SERVER_SRC)/ftpnew.c

CLIENT_SRCS := test_client.c \
               $(COMMON_SRCS) \
               $(CLIENT_SRC)/ftpnew_client.c \
               $(CLIENT_SRC)/ftpnew_client_crc32.c

all: $(BUILDDIR)/test_server $(BUILDDIR)/test_client

ifeq ($(KEEP_OBJ),true)
# Two-step: per-source .o under $(BUILDDIR)/, then link.

COMMON_OBJS := $(BUILDDIR)/test_common.o \
               $(BUILDDIR)/ftpnew_utils.o

SERVER_OBJS := $(BUILDDIR)/test_server.o \
               $(COMMON_OBJS) \
               $(BUILDDIR)/ftpnew.o

CLIENT_OBJS := $(BUILDDIR)/test_client.o \
               $(COMMON_OBJS) \
               $(BUILDDIR)/ftpnew_client.o \
               $(BUILDDIR)/ftpnew_client_crc32.o

$(BUILDDIR)/test_server: $(SERVER_OBJS) | $(BUILDDIR)
	$(CC) $(SERVER_OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILDDIR)/test_client: $(CLIENT_OBJS) | $(BUILDDIR)
	$(CC) $(CLIENT_OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

# Local sources (test_common.c, test_server.c, test_client.c).
$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Server sources.
$(BUILDDIR)/%.o: $(SERVER_SRC)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Client sources.
$(BUILDDIR)/%.o: $(CLIENT_SRC)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

else
# One-shot compile+link: gcc handles compilation with internal temp
# objects (under /tmp), then links and deletes the temps. Nothing on disk
# under the source tree or under build/ except the final binaries.

$(BUILDDIR)/test_server: $(SERVER_SRCS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(SERVER_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILDDIR)/test_client: $(CLIENT_SRCS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(CLIENT_SRCS) -o $@ $(LDFLAGS) $(LDLIBS)

endif

$(BUILDDIR):
	mkdir -p $@

$(INSTALL_LIB_DIR):
	mkdir -p $@

# Bundles the runtime libs alongside the binaries so build/$(TARGET)/
# is a self-contained deploy unit. -P preserves symlinks (libfoo.so ->
# libfoo.so.1) so SONAME-style versioning survives a copy.
install: all $(addprefix $(INSTALL_LIB_DIR)/,$(INSTALL_LIBS))
	@echo "[install] runtime libs in $(INSTALL_LIB_DIR)"

$(INSTALL_LIB_DIR)/%.so: $(EXT_LIB_DIR)/%.so | $(INSTALL_LIB_DIR)
	cp -P $< $@

# 'clean' also sweeps any stray .o that may have been left by older
# Makefile versions next to the sources.
clean:
	rm -rf build server.log
	rm -f *.o $(SERVER_SRC)/*.o $(CLIENT_SRC)/*.o

.PHONY: all clean install
