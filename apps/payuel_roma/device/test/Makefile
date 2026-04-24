CC       := gcc
CFLAGS   := -Wall -Wextra
LDFLAGS  :=

DEVICE_INC := ../inc
DEVICE_SRC := ../src
SIMDEV_SRC := ./simdev
UTILS_SRC  := ./common

TEST_DIR   := .
TEST_LIBS  := $(TEST_DIR)/libs
BUILD_DIR  := exe

INCLUDES := -I$(DEVICE_INC) -I$(TEST_DIR) -I$(UTILS_SRC)

LDFLAGS += -L$(TEST_LIBS) -lcsp -lpthread  #-lssap

DEVICE_SRCS := $(wildcard $(DEVICE_SRC)/*.c)
DEVICE_OBJS := $(patsubst $(DEVICE_SRC)/%.c,$(BUILD_DIR)/device/%.o,$(DEVICE_SRCS))

SIMDEV_SRCS := $(wildcard $(SIMDEV_SRC)/*.c)
SIMDEV_OBJS := $(patsubst $(SIMDEV_SRC)/%.c,$(BUILD_DIR)/%.o,$(SIMDEV_SRCS))

UTILS_SRCS := $(wildcard $(UTILS_SRC)/*.c)
UTILS_OBJS := $(patsubst $(UTILS_SRC)/%.c,$(BUILD_DIR)/%.o,$(UTILS_SRCS))

TEST_SRCS := $(wildcard $(TEST_DIR)/s5lab_*.c)
TEST_BINS := $(patsubst $(TEST_DIR)/s5lab_%.c,$(BUILD_DIR)/%,$(TEST_SRCS))

.PHONY: all clean
 
all: $(TEST_BINS) $(BUILD_DIR)/simdev_server
 
# Link: one binary per test suite
$(BUILD_DIR)/%: $(BUILD_DIR)/s5lab_%.o $(DEVICE_OBJS) $(UTILS_OBJS)
	@echo "utilobj =  $(UTILS_OBJS)"
	@echo "utilsrc =  $(DEVICE_SRCS)"
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/simdev_server: $(SIMDEV_OBJS) $(UTILS_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR) $(BUILD_DIR)/device:
	mkdir -p $@

# Compile: driver sources
$(BUILD_DIR)/device/%.o: $(DEVICE_SRC)/%.c | $(BUILD_DIR)/device
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
 
# Compile: test sources
$(BUILD_DIR)/s5lab_%.o: $(TEST_DIR)/s5lab_%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
 
# Compile: simulator sources
$(BUILD_DIR)/%.o: $(SIMDEV_SRC)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Compile: utils sources
$(BUILD_DIR)/%.o: $(UTILS_SRC)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)
