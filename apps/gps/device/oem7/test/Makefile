#CC=arm-linux-gnueabi-gcc
CC=gcc
TGT=oem_driver_tester

SRC=../src/oem_task.c \
    ../src/oem_cmd.c \
    ../src/oem_log.c \
    ../src/oem_io.c \
    ../src/oem_utils.c \
    ../src/io_drivers/oem_io_serial_linux.c \
    ../callbacks/oem_cb.c \
	../callbacks/oem_cb_version.c \
	../callbacks/oem_cb_hwmonitor.c \
	../callbacks/oem_cb_bestxyz.c \
	oem_test.c

IFLAGS=-I ../inc \
       -I ../inc/msg \
	   -I ../src \
	   -I ../src/arch \
	   -I ../callbacks
	
LIBS=-lpthread
WOPT := -W -Wall -Wshadow -Wpointer-arith -Wcast-qual

.PHONY=all clean

all: $(TGT)

clean:
	rm $(TGT)

$(TGT): $(SRC)
	$(CC) -o $@ $(SRC) $(IFLAGS) $(WOPT) $(LIBS)
