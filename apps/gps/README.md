# BEE-1012 GPS app drop-in

Replaces `FSW_BEE1012/apps/gps/` wholesale. Built from the sd1 GPS app, with
the report contract, command codes, config layout and entry symbol changed
back to what BEE-1012 already expects.

Not part of the NovAtel_OEM719 history — this directory is a staging area, not
a tracked source of truth.

## Install

    rm -rf FSW_BEE1012/apps/gps
    cp -r bee1012_gps_drop FSW_BEE1012/apps/gps

Nothing outside `apps/gps/` needs to change:

| Already correct in BEE-1012 | Why it still works |
|---|---|
| `bee1012_defs/*_cfe_es_startup.scr` | entry symbol is `GPS_AppMain` |
| `bee1012_defs/targets.cmake` | `gps` is already in `MISSION_GLOBAL_APPLIST` |
| `apps/rpt/fsw/tables/rpt_tbl.c` | report MID is still named `GPS_REPORT_TLM_MID` |
| ground command definitions | every `*_CC` keeps its BEE-1012 number |
| `GPS_CMD_MID` / `GPS_HK_TLM_MID` | topic IDs 0x94 / 0x95 unchanged |

## What is new

**Eight stored-log telemetry packets.** The receive task's callbacks publish
BESTXYZ, BESTPOS, RANGE, TIME, CLOCKMODEL, HWMONITOR, RXSTATUS and SATVIS2 on
telemetry topic IDs 0x9A - 0xA1 (MIDs 0x089A - 0x08A1), a block that was free
in the mission's allocation. Nothing subscribes to them yet: add the ones the
mission wants to `apps/ds/fsw/tables/ds_filter_tbl.c` to record them, and to
`apps/to_lab/fsw/tables/to_lab_sub.c` to downlink them.

**Housekeeping on request.** `GPS_DRIVER_REPORT_HK_CC` (4) was declared in
BEE-1012 but never dispatched. It now returns the housekeeping payload in a
command report, which is the only telemetry route out of this app that the
mission actually carries - SCH sends no `GPS_SEND_HK_MID`, and no table
subscribes to `GPS_HK_TLM_MID`. The periodic HK packet still works if the
mission ever wires it.

**A real housekeeping payload.** `GPS_HkTlm_Payload_t` was `{ uint32 dummy; }`.
It now carries the command counters and the receive-path counters bucketed by
the layer that gave up (see `oem_task_context_t.task_level`).

**The current OEM7 driver.** `device/oem7/` is the NovAtel_OEM719 master copy:
larger parser and I/O buffers so RANGE and SATVIS2 fit, RXSTATUS / SATVIS2 /
BESTPOS log structures, bounds checks on the variable-length log callbacks, and
the accumulated parser fixes. `device/oem7/test/` is not included.

## Behaviour changes to know about

- Callbacks load through OSAL (`OS_ModuleLoad` / `OS_SymbolLookup`) instead of
  `dlopen`/`dlsym`, and the module path must sit under `GPS_PLATFORM_MODULE_DIR`
  ("/cf/"). The old code called `dlclose()` before the loaded callback ran.
- `GPS_OEM_LOG_LOCK_HANDLERS_CC` / `..._UNLOCK_...` validate their magic key and
  then report `GPS_ERR_UNIMPL`. The driver exposes no handler lock;
  `oem_log_lock_handlers()` is commented out in `oem_log.h`. They used to
  return silently, which ground read as success.
- BESTXYZ no longer appends to `/cf/sdcard/bestxyz.bin`; it goes out as
  telemetry.
- The serial device, baud, task priority and stack are in
  `config/default_gps_internal_cfg.h` instead of hard-coded in the source.
- App shutdown waits for the receive task to leave the driver before closing
  the port (`GPS_PLATFORM_RX_JOIN_MS`, 12 s).

## Verification status

Compiles clean under the flags BEE-1012 uses (`-std=c99 -pedantic -Wall
-Wstrict-prototypes -Wwrite-strings -Wpointer-arith -Werror`), 22 sources, zero
warnings, against **BEE-1012's own generated headers** (`make prep`, Equuleus
cFE), for both x86_64 and 32-bit ARM - the flight target's word size.

Two caveats:

- Only a compile, not a link or a run. `make prep` stops before the app targets
  because `_deps/libaec` demands CMake >= 3.26 and this machine has 3.16.3.
  That is an unrelated dependency of another app; the mission and platform
  headers this check needs were all generated before it failed.
- Not built as `apps/gps`. The generated `gps_*.h` bridges point at whatever
  sits in `apps/gps/config`, so they were overridden to point at this
  directory's `config/`. Everything else - cFE, OSAL, PSP and the RPT contract -
  came from BEE-1012's real generated tree.

Run a full `make` once the CMake version allows it.
