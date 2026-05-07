# MEOW — Mission Executive for Orbit Welfare


**MEOW** is a cFS application that provides ground-commandable access to the Linux environment running on the satellite. Operators can **execute shell commands**, **surgically manage files**, **query system state**, and **trigger emergency shutdown or kill sequences**.

MEOW was originally created in 2023 (which then stood for "MIMAN Emergency Operations Wrapper") as a temporary application for solving the unmounted SD card problem during the MIMAN mission operations. *Now it's reborn*, still to serve as a helper for emergency onboard managements.

## Architecture

The active logic lives in `fsw/src/core/` as pure POSIX C with no cFS dependencies:

| Module | Purpose |
|---|---|
| `meow_shell` | Shell command execution (sync/async), timeout, kill, blacklist |
| `meow_file` | File read/write/copy/move/stat/tail, CRC-32, disk stat |
| `meow_sys` | sysinfo, time get/set, sync, shutdown, force-kill |
| `meow_csp` | CSP node, server thread, port dispatch, FTP, dynamic handler loading |


The CSP module is an optional component: wiring the CSP module requires libcsp and GS FTP integration.

## Tests

Unit tests (safe, in-process):
```bash
# From the cFS repo root
make SIMULATION=native ENABLE_UNIT_TESTS=true prep && make && make test
```

Danger tests (destructive behavior, Docker-isolated):
```bash
./unit-test/danger/run_danger_tests.sh
```
