# EPS Application

This repository contains a resusable EPS (Electrical Power System) Application,
which includes driver support for the GomSpace P31u and P60 power systems.
Migrated to the latest unreleased version of the Core Flight Executive (cFE)
framework (equuleus).


# Status

2025-07-09: Application compiles. Functionality not yet tested on hardware.


# TODOs

- Add compile-time device selection.
- Refactor Event Service messages and EIDs.
- Implement full closure for the dispatching function.
- Define a minimal housekeeping message format.
- Add an abstraction layer for the ground command reports.
- Hardware testing.
