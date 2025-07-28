# RPT (Report) App user guide
- Author : Hyeok-jin Kweon
- Department of Satellite Systems, Yonsei University

# Update
- 25/07/28 : Initial version

# Overview
- This app provides report functionality API for General Mission
    1. **Core Operation data** of spacecraft
    2. Result of Ground Command (TC)
    3. Reporting for critical Satellite telemetry

# Functionality
## 1. Core Operation data
- RPT track the some operation data (e.g. Boot count, spacecraft time...etc) and Store it to NON-volatile memory
- And automatically load the latest data after reboot
- Backup Operation data is stored periodically
- CRC32 is used for data validation

## 2. Result of TC
- RPT subscribe various MID, and ingest various Report packet.
- Ingested packet will be enqueued to RPT FIFO, and also publish the message
- TO will ingest this and send it to GS
- GS can request the report as they want by Ground command

## 3. Reporting for critical telemetry
- RPT has another FIFO, which store the Critical telemetry
- Critical FIFO has not only all normal report data, but also time information
- This Critical telemetry will be store to NON-volatile memory
- GS can request the critical report as they want by Ground command

# How to Use
## 0. Edit the RPT app Table
- Firstly, user should edit the RPT app Table, which located at `rpt/fsw/tables/rpt_tbl.c`
- Structure of the table is writed at below
```c
RPT_Table_t RPT_Subs[RPT_MAX_TBL_ENTRY] = {
    /* Entry 0 */
    {.UsedState = RPT_ENABLED,
        .Entry.MessageID = CFE_SB_MSGID_WRAP_VALUE(0x0823),
        .Entry.IsCritical = RPT_CRITICAL},

    /* Entry 1 */
    {.UsedState = RPT_DISABLED,
        .Entry.MessageID = CFE_SB_MSGID_RESERVED,
        .Entry.IsCritical = RPT_NOT_CRITICAL},

    /* More Entry */
    // .......
}
```
1. UsedState : Indicate whether the entry will be used or not.
    - If used, set to `RPT_ENABLED`
    - If not, set to `RPT_DISABLED`
2. MessageID : Message ID which RPT subscribe to.
    - The Message ID (MID) of the report message can be assigned freely.
    - But also, transmited Report Message should be initialized to this MID
3. IsCritical : Flag which indicate whether the message is critical or not
    - If critical, set to `RPT_CRITICAL`
    - If not, set to `RPT_NOT_CRITICAL`

## 1. Configure Report Packet
- After the table reivision, external app should configure the Report packet. The structure is shown below
```c
typedef struct RPT_Report {
    uint16_t MsgID;
    uint8_t CommandCode;

    /**
     * `enum RPT_ReturnType_t`
     */
    uint8_t ReturnType;
    int32_t ReturnCode;

    /**
     * This member can be used to 2 types
     * 1. Get CMD -> earned value
     * 2. HW's Error packet -> HW error value
     */
    uint16_t ReturnDataSize;
    uint8_t ReturnValue[RPT_RET_VALUE_BUF_SIZE]; /* `RPT_RET_VALUE_BUF_SIZE` == 60 */
} RPT_Report_t;
```
1. MsgID : Specific Message ID which the satellite ingest
2. CommandCode : Specific Command code which the satellite ingest
3. ReturnType : Returned code type. Refer the `default_rpt_interface_cfg.h`
    ```c
    /********************************
     * Return Type definition
    ********************************/
    typedef enum {
        RPT_RETCODE_SUCCESS,
        /**
        * `osapi-error.h`
        */
        RPT_RETCODE_OSAL,

        /**
        * `cfe_error.h`
        */
        RPT_RETCODE_CFE,
        RPT_RETCODE_LIB,
        RPT_RETCODE_APP,
        RPT_RETCODE_APPUTIL,
        RPT_RETCODE_KNL, /* Kernel layer */

        /**
        * Use this if HW send error packet
        */
        RPT_RETCODE_HW,

    } RPT_ReturnType_t;
    ```
4. ReturnCode : Returned code (i.e. Return value of particular function)
5. ReturnDataSize : If invoked function return some telemetry value, put the returned data size of the value
6. ReturnValue : If invoked function return some telemetry value, put the specific value

## 2. Transmit Message
- After the packet configuration, user app should transmit the message to SB (Software Bus)
- Set the MID of the report message same as the RPT table