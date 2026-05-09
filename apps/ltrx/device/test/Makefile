CC=gcc
CFLAGS=-Wall -Wextra -pthread

SRC_LTRX_IFC=../ifc/ltrx_ifc_serial_linux.c
SRC_LTRX=../src/ltrx.c
SRC_TEST=ltrx_test_serial.c ltrx_test_utils.c
IFLAGS=-I../inc -I../ifc -I.

all: ltrx_test
ltrx_test: $(SRC_LTRX_IFC) $(SRC_LTRX) $(SRC_TEST)
	$(CC) $(CFLAGS) $(IFLAGS) -o ltrx_test $(SRC_LTRX_IFC) $(SRC_LTRX) $(SRC_TEST)
clean:
	rm -f ltrx_test