CC=arm-linux-gnueabi-gcc
#CC=gcc
TGT=oem_driver_tester

SRC=../src/oem_task.c \
    ../src/oem_cmd.c \
    ../src/oem_log.c \
    ../src/oem_io.c \
    ../src/oem_utils.c \
    ../src/arch/oem_arch_io_linux.c \
    ../callbacks/oem_log_cb.c \
    ../mlk/list/src/mlk_list.c \
	oem_test.c

IFLAGS=-I ../inc \
       -I ../inc/msg \
	   -I ../src \
	   -I ../src/arch \
	   -I ../mlk \
	   -I ../mlk/list/inc \
	   -I ../mlk/list/src \
	   -I ../callbacks
	
LIBS=-lpthread
WOPT := -W -Wall -Wshadow -Wpointer-arith -Wcast-qual

.PHONY=all clean

all: $(TGT)

clean:
	rm $(TGT)

$(TGT): $(SRC)
	$(CC) -o $@ $(SRC) $(IFLAGS) $(WOPT) $(LIBS)
