# PAYUEL_OBC

`payuel_obc` is the cFS app that talks to the UEL payload OBC node over CSP/CAN.

Authoritative interface source:
- [UELYSYS_OBC-ICD_V0.2.xlsx](/home/eodnjs7783/dss/0413/PAYUEL/UELYSYS_OBC-ICD_V0.2.xlsx)

Secondary summary only:
- [UELYSYS_Payload_Development_Summary_V1.1.pdf](/home/eodnjs7783/dss/0413/PAYUEL/UELYSYS_Payload_Development_Summary_V1.1.pdf)

Command map used by this app:
- `0x20`: UEL OBC beacon request
- `0x21`: motor mode control
- `0x41`: capture request
- `0x44`: image download metadata
- `0x45`: image chunk download
- `0x54`: sensor download metadata
- `0x55`: sensor chunk download

Notes:
- `PAYUEL_OBC_CMD_MID` carries operator-issued ground commands.
- `PAYUEL_OBC_SEND_BCN_MID` is a scheduler/internal trigger for the same beacon path.
- `0x41`, `0x44`, and `0x45` also exist in `payuel_cam`, but they are not collisions because each app targets a different payload node and has its own SB MIDs.
- Legacy SPI receive/init snippets are preserved as commented reference blocks, but the active implementation uses CSP/CAN only.
