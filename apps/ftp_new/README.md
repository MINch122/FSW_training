
# FTP-NEW

> **F**\**king **T**ired of **P**atching these **N**onsensical **E**xtensions and **W**rappers for FTP

So. Tired.

**Version 1.0**


## Features
### Server backend
- File upload/download handlers compatible with the original client
- Error fixes from the original backend (e.g., zero-sized file/chunk)
- UDP connection support with a configurable interpacket delay
- Partial file download with offset and size
- Queued handler error logs retrievable by the client
- Extension layer requests (logs, shell commands, ram access, etc.)

### Client
- UDP connection support with a configurable interpacket delay
- Extension layer transaction APIs

### Unit test
- Self-contained server-client transaction test suite (uses a VCAN interface)


## Usage
Server binary build:
```bash
cd server && make               # Native linux
cd server && make TARGET=arm32  # ARMel
```

Unit test build:
```bash
cd test && make install              # Native linux
cd test && make install TARGET=arm32 # ARMel
```

Unit test run:
```bash
cd test && sudo ./run_tests.sh              # Native linux
cd test && sudo ./run_tests.sh TARGET=arm32 # ARMel
```

Server deploy:

Call `ftp_server_start()` after a proper CSP initialization.


## Dependencies
- `libcsp.so`
- `gs_error_t` and legacy FTP packet definitions
- VCAN interface (unit test only)
