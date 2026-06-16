#!/usr/bin/env bash
# Bring up vcan0, launch test_server in the background, run test_client,
# then tear everything down.
#
# Usage:
#   ./run_tests.sh                     # native build
#   TARGET=arm32 ./run_tests.sh        # only useful when running on ARM
#   VCAN=vcan1   ./run_tests.sh        # use a different vcan interface
#
# Requires sudo for the modprobe / ip link steps; no-ops if the interface
# is already up.
#
# The binaries must already be built. Run e.g. 'make TARGET=$TARGET' first.

set -e

TARGET=${TARGET:-native}
VCAN=${VCAN:-vcan0}

# Binaries live under build/$TARGET/ after the Makefile rewrite.
SERVER=build/$TARGET/test_server
CLIENT=build/$TARGET/test_client

# Per-process working dirs. test_server's prep_workdir() chdirs to its
# own cwd (or $FTPNEW_TEST_WORKDIR if set); test_client creates ./ftpn_c
# and ./ftpn_s relative to its cwd. We launch both from this directory
# (test/) so they share the same view of those subdirs.
SERVER_FILES_DIR=./ftpn_s
CLIENT_FILES_DIR=./ftpn_c

bring_up_vcan() {
    # Need three modules:
    #   can      - core CAN networking subsystem
    #   can_raw  - PF_CAN/SOCK_RAW socket family (libcsp uses this)
    #   vcan     - the virtual-CAN driver that backs vcan0
    # Without can_raw, socket(PF_CAN, SOCK_RAW, CAN_RAW) returns
    # EPROTONOSUPPORT even when vcan0 exists.
    echo "[setup] loading CAN kernel modules"
    sudo modprobe can       2>/dev/null || true
    sudo modprobe can_raw   2>/dev/null || true
    sudo modprobe vcan      2>/dev/null || true

    if ip link show "$VCAN" >/dev/null 2>&1; then
        echo "[setup] $VCAN already exists"
    else
        echo "[setup] creating $VCAN"
        sudo ip link add dev "$VCAN" type vcan
    fi
    sudo ip link set up "$VCAN"
}

cleanup() {
    if [ -n "$SERVER_PID" ] && kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "[cleanup] stopping server (pid=$SERVER_PID)"
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT

# ---- Pre-flight ----------------------------------------------------------

if [ ! -x "$SERVER" ] || [ ! -x "$CLIENT" ]; then
    echo "[fatal] missing binaries for TARGET=$TARGET"
    echo "        expected: $SERVER"
    echo "        expected: $CLIENT"
    echo "        run 'make TARGET=$TARGET' first"
    exit 1
fi

bring_up_vcan

# ---- Per-run scratch dirs -----------------------------------------------

rm -rf "$SERVER_FILES_DIR" "$CLIENT_FILES_DIR"
mkdir -p "$SERVER_FILES_DIR" "$CLIENT_FILES_DIR"

# ---- Launch -------------------------------------------------------------

echo "[run] starting server ($SERVER)"
"$SERVER" > server.log 2>&1 &
SERVER_PID=$!
sleep 1

if ! kill -0 "$SERVER_PID" 2>/dev/null; then
    echo "[fatal] server did not come up; see server.log:"
    cat server.log
    exit 1
fi

echo "[run] running client ($CLIENT)"
"$CLIENT"
RC=$?

echo "[run] client exit code: $RC"
exit $RC
