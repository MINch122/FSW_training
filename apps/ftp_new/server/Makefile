# Build the ftpnew server as a shared library.
#
# Usage:
#   make                        # native, one-shot compile+link, no .o files
#   make TARGET=arm32           # cross-build for ARM32
#   make KEEP_OBJ=true          # also emit per-source .o under build/<target>/
#
# Per-target outputs land in build/$(TARGET)/. Both targets coexist. The
# final .so is size-optimized (-Os), dead-section-stripped, and run through
# 'strip --strip-unneeded' to drop everything but the dynamic symbol table.


TARGET   ?= native
KEEP_OBJ ?= false

ifeq ($(TARGET),native)
    CC    := gcc
    STRIP := strip
else ifeq ($(TARGET),arm32)
    CC    := arm-linux-gnueabi-gcc
    STRIP := arm-linux-gnueabi-strip
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

# -Os                            : optimize for size, not speed
# -ffunction-sections / -fdata-sections : per-symbol ELF sections so the
#                                  linker can drop unreferenced ones
# -fPIC                          : required for shared library code
CFLAGS       = -Wall -Wextra -Os -fPIC \
               -ffunction-sections -fdata-sections \
               -I$(INCDIR) \
               -I$(CONFDIR) \
               -I$(EXT_INC)

# --gc-sections   : drop unreferenced per-symbol sections (paired with
#                   -ffunction/-fdata-sections above)
# --as-needed     : omit DT_NEEDED entries for libs whose symbols nothing
#                   in the output actually references
LDFLAGS      = -shared -Wl,-soname,$(SO_FILE) \
               -Wl,--gc-sections -Wl,--as-needed \
               -L$(EXT_LIB_DIR)

LIBS         = -lcsp -lpthread

OUT          = $(BUILDDIR)/$(SO_FILE)

all: $(OUT)

ifeq ($(KEEP_OBJ),true)
# Two-step: per-source .o under $(BUILDDIR)/, then link.

OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

$(OUT): $(OBJS) | $(BUILDDIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) --strip-unneeded $@
	ln -sf $(SO_FILE) $(BUILDDIR)/$(SO_NAME)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

else

$(OUT): $(SRCS) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(LIBS)
	$(STRIP) --strip-unneeded $@
	ln -sf $(SO_FILE) $(BUILDDIR)/$(SO_NAME)

endif

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf build
	rm -f $(SRCDIR)/*.o

.PHONY: all clean
