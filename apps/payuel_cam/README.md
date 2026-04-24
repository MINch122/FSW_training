# PAYUEL_CAM

`payuel_cam` is the cFS app that talks to the CAM interface board over CSP/CAN.

Authoritative interface source:
- [CAM_INTERFACE_BOARD-ICD_v0.2_260303.xlsx](/home/eodnjs7783/dss/0413/PAYUEL/CAM_INTERFACE_BOARD-ICD_v0.2_260303.xlsx)

Secondary summary only:
- [UELYSYS_Payload_Development_Summary_V1.1.pdf](/home/eodnjs7783/dss/0413/PAYUEL/UELYSYS_Payload_Development_Summary_V1.1.pdf)

Command map used by this app:
- `0x41`: capture (RAW 생성)
- `0x42`: healthcheck
- `0x43`: postprocess
- `0x44`: download metadata
- `0x45`: download chunk
- `0x46`: beacon

Notes:
- The spreadsheet ICD is authoritative when terminology differs from the summary PDF.
- For `0x43`/`0x44`/`0x45`, this code follows the ICD parameter naming used in the spreadsheet.
- `0x45` keeps the spreadsheet wire format: request CRC16, fixed 256-byte response, CRC32 over valid data only.
- Single-chunk `0x45` stays synchronous; only the aggregate full-image download runs in a child task.
- `PAYUEL_CAM_CMD_MID` carries operator-issued ground commands.
- `PAYUEL_CAM_SEND_BCN_MID` is a scheduler/internal trigger for the same beacon path.
