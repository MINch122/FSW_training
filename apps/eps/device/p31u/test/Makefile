# CC=arm-linux-gnueabi-gcc
CC=gcc
TGT=p31u_driver_tester

SRC=../src/p31u.c \
	../src/ifc/p31u_i2c_linux.c \
	p31u_test_handler.c \
	p31u_test.c
IFLAGS=-I../inc
LIBS=-lreadline
WOPT := -W -Wall -Wshadow -Wpointer-arith -Wcast-qual

.PHONY=all clean

all: $(TGT)

clean:
	rm $(TGT)

$(TGT): $(SRC)
	$(CC) -o $@ $(SRC) $(IFLAGS) $(LIBS) $(WOPT)
