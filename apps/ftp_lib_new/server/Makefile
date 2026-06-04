# Build the ftpnew server as a shared library.
#
# Usage:
#   make                        # native, one-shot compile+link, no .o files
#   make TARGET=arm32           # cross-build for ARM32
#   make KEEP_OBJ=true          # also emit per-source .o under build/<target>/
#
# Per-target outputs land in build/$(TARGET)/. Both targets coexist.


TARGET   ?= native
KEEP_OBJ ?= false

ifeq ($(TARGET),native)
    CC := gcc
else ifeq ($(TARGET),arm32)
    CC := arm-linux-gnueabi-gcc
else
    $(error TARGET must be 'native' or 'arm32' (got '$(TARGET)'))
endif

EXT_DIR     := ../test/external
EXT_INC     := $(EXT_DIR)/include
EXT_LIB_DIR := $(EXT_DIR)/lib/$(TARGET)

SO_NAME      = libftpnew_server.so
SO_VER       = 1
SO_FILE      = $(SO_NAME).$(SO_VER)

SRCDIR       = src
INCDIR       = inc
CONFDIR      = ../config
BUILDDIR     = build/$(TARGET)

SRCS         = $(SRCDIR)/ftpnew.c $(SRCDIR)/ftpnew_utils.c

CFLAGS       = -Wall -Wextra -O2 -fPIC \
               -I$(INCDIR) \
               -I$(CONFDIR) \
               -I$(EXT_INC)

LDFLAGS      = -shared -Wl,-soname,$(SO_FILE) \
               -L$(EXT_LIB_DIR)

LIBS         = -lcsp -lpthread

OUT          = $(BUILDDIR)/$(SO_FILE)

all: $(OUT)

ifeq ($(KEEP_OBJ),true)
# Two-step: per-source .o under $(BUILDDIR)/, then link.

OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

$(OUT): $(OBJS) | $(BUILDDIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	ln -sf $(SO_FILE) $(BUILDDIR)/$(SO_NAME)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

else

$(OUT): $(SRCS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(LIBS)
	ln -sf $(SO_FILE) $(BUILDDIR)/$(SO_NAME)

endif

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf build
	rm -f $(SRCDIR)/*.o

.PHONY: all clean
