#include "lgbat_task.h"
#include "lgbat_cmds.h"
#include "lgbat_eventids.h"
#include "lgbat_msgids.h"
#include "osapi.h"



// Local helpers


// Read a 2-byte big-endian value from a byte buffer and return as uint16.
static inline uint16_t LGBAT_BE16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

// Read a 2-byte big-endian value from a byte buffer and return as signed int16.
static inline int16_t LGBAT_BE16S(const uint8_t *p)
{
    return (int16_t)(((uint16_t)p[0] << 8) | p[1]);
}

// Verify XOR checksum of a 10-byte BMS response buffer.
// The checksum byte is the last byte; the XOR of all previous bytes must equal it.
static inline bool LGBAT_VerifyChecksum(const uint8_t *Buf, uint8_t Len)
{
    if (Buf == NULL || Len < 2) return false;
    uint8_t xorVal = 0;
    for (uint8_t i = 0; i < (Len - 1); i++) xorVal ^= Buf[i];
    return (xorVal == Buf[Len - 1]);
}

// Return a human-readable name string for a BMS Data ID.
static const char *LGBAT_GetDataIdName(uint8_t DataID)
{
    switch (DataID)
    {
        case LGBAT_DATA_ID_POWER_TIME:       return "Pack power and time";
        case LGBAT_DATA_ID_CAPACITY_STATUS:  return "Capacity and status";
        case LGBAT_DATA_ID_BATTERY_INFO:     return "Battery information";
        case LGBAT_DATA_ID_CHARGE_STATE:     return "Charge state";
        case LGBAT_DATA_ID_CELL_EXTREMES:    return "Cell voltage and temperature extremes";
        case LGBAT_DATA_ID_CELL_VOLTAGE:     return "Cell voltages";
        case LGBAT_DATA_ID_CELL_TEMPERATURE: return "Cell temperatures";
        case LGBAT_DATA_ID_FET_STATUS:       return "FET temperatures and software version";
        case LGBAT_DATA_ID_BMS_STATUS:       return "BMS status";
        case LGBAT_DATA_ID_FAIL_STATUS:      return "Failure status";
        case LGBAT_DATA_ID_MCU_VOLTAGE:      return "Boost and MCU voltages";
        case LGBAT_DATA_ID_DCDC_STATUS:      return "PCB current and boost status";
        default:                             return "Unknown DataID";
    }
}


// RPT helper: send one command report telemetry packet.

static void LGBAT_SendReport(uint16_t MsgID, uint8_t CC, CFE_Status_t RetCode,
                             const void *Data, uint32_t DataSize)
{
    RPT_Report_t report;
    memset(&report, 0, sizeof(report));
    report.MsgID          = MsgID;
    report.CommandCode    = CC;
    report.ReturnType     = (RetCode == CFE_SUCCESS) ? RPT_RETTYPE_SUCCESS : RPT_RETTYPE_CFE;
    report.ReturnCode     = RetCode;
    report.ReturnDataSize = 0;

    if (Data != NULL && DataSize > 0)
    {
        uint32_t copySize = (DataSize > RPT_RET_VALUE_BUF_SIZE)
                            ? RPT_RET_VALUE_BUF_SIZE : DataSize;
        memcpy(report.ReturnValue, Data, copySize);
        report.ReturnDataSize = (uint16_t)copySize;
    }

    LGBAT_Data.ReportTlm.Payload = report;
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(LGBAT_Data.ReportTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(LGBAT_Data.ReportTlm.TelemetryHeader), true);
}


// OS_printf helpers for ICD-format log output


// Print the raw I2C receive buffer in [i]=0xXX format.
static void LGBAT_PrintRawRxData(uint8_t DataID, const uint8_t *Buf, uint8_t Len)
{
    if (Buf == NULL || Len == 0) return;
    OS_printf("LGBAT I2C RX [0x%02X %s]:", DataID, LGBAT_GetDataIdName(DataID));
    for (uint8_t i = 0; i < Len; i++) OS_printf(" [%u]=0x%02X", i, Buf[i]);
    OS_printf("\n");
}

// Print parsed field values for each Data ID in ICD-aligned format.
static void LGBAT_PrintParsedData(uint8_t DataID)
{
    switch (DataID)
    {
        // Data ID 0x01: Pack Power and Time
        case LGBAT_DATA_ID_POWER_TIME:
            OS_printf("LGBAT I2C [0x01 %s]:"
                      " Pack_Voltage=%u Pack_Current=%d"
                      " Average_Time_To_Empty=%u Average_Time_To_Full=%u CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data01.Pack_Voltage,
                      LGBAT_Data.BmsData.Data01.Pack_Current,
                      LGBAT_Data.BmsData.Data01.Average_Time_To_Empty,
                      LGBAT_Data.BmsData.Data01.Average_Time_To_Full,
                      LGBAT_Data.BmsData.Data01.CheckSum);
            break;

        // Data ID 0x02: Capacity and Status
        case LGBAT_DATA_ID_CAPACITY_STATUS:
            OS_printf("LGBAT I2C [0x02 %s]:"
                      " Power_Supply_Status=%u SOH=%u SOC=%u"
                      " RC_Remaining_Capacity=%u AE_Available_Energy=%u CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data02.Power_Supply_Status,
                      LGBAT_Data.BmsData.Data02.SOH,
                      LGBAT_Data.BmsData.Data02.SOC,
                      LGBAT_Data.BmsData.Data02.RC_Remaining_Capacity,
                      LGBAT_Data.BmsData.Data02.AE_Available_Energy,
                      LGBAT_Data.BmsData.Data02.CheckSum);
            break;

        // Data ID 0x03: Battery Information
        case LGBAT_DATA_ID_BATTERY_INFO:
            OS_printf("LGBAT I2C [0x03 %s]:"
                      " System_Max_Voltage=%u Power_Supply_Health=%u"
                      " FETTestRequiredVoltage=%u"
                      " Reserved={0x%02X,0x%02X,0x%02X} CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data03.System_Max_Voltage,
                      LGBAT_Data.BmsData.Data03.Power_Supply_Health,
                      LGBAT_Data.BmsData.Data03.FETTestRequiredVoltage,
                      LGBAT_Data.BmsData.Data03.Battery_Information_Reserved[0],
                      LGBAT_Data.BmsData.Data03.Battery_Information_Reserved[1],
                      LGBAT_Data.BmsData.Data03.Battery_Information_Reserved[2],
                      LGBAT_Data.BmsData.Data03.CheckSum);
            break;

        // Data ID 0x04: Charge State
        case LGBAT_DATA_ID_CHARGE_STATE:
            OS_printf("LGBAT I2C [0x04 %s]:"
                      " Percentage=%u Design_Capacity=%u"
                      " Capacity=%u Charge=%u CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data04.Percentage,
                      LGBAT_Data.BmsData.Data04.Design_Capacity,
                      LGBAT_Data.BmsData.Data04.Capacity,
                      LGBAT_Data.BmsData.Data04.Charge,
                      LGBAT_Data.BmsData.Data04.CheckSum);
            break;

        // Data ID 0x05: Cell Voltage and Temperature Extremes
        case LGBAT_DATA_ID_CELL_EXTREMES:
            OS_printf("LGBAT I2C [0x05 %s]:"
                      " Cell_Voltage_Max=%u Cell_Voltage_Min=%u"
                      " Cell_Temperature_Max=%d Cell_Temperature_Min=%d CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data05.Cell_Voltage_Max,
                      LGBAT_Data.BmsData.Data05.Cell_Voltage_Min,
                      LGBAT_Data.BmsData.Data05.Cell_Temperature_Max,
                      LGBAT_Data.BmsData.Data05.Cell_Temperature_Min,
                      LGBAT_Data.BmsData.Data05.CheckSum);
            break;

        // Data ID 0x06: Cell Voltages
        case LGBAT_DATA_ID_CELL_VOLTAGE:
            OS_printf("LGBAT I2C [0x06 %s]:"
                      " Cell_Voltage_01=%u Cell_Voltage_02=%u"
                      " Reserved_01=%u Reserved_02=%u CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data06.Cell_Voltage_01,
                      LGBAT_Data.BmsData.Data06.Cell_Voltage_02,
                      LGBAT_Data.BmsData.Data06.Reserved_01,
                      LGBAT_Data.BmsData.Data06.Reserved_02,
                      LGBAT_Data.BmsData.Data06.CheckSum);
            break;

        // Data ID 0x07: Cell Temperatures
        // ICD byte order: byte[1-2]=T02, byte[3-4]=T01
        case LGBAT_DATA_ID_CELL_TEMPERATURE:
            OS_printf("LGBAT I2C [0x07 %s]:"
                      " Cell_Temperature_01=%d Cell_Temperature_02=%d"
                      " Balancing_R_Temperature=%d PreCharge_R_Temperature=%d CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data07.Cell_Temperature_01,
                      LGBAT_Data.BmsData.Data07.Cell_Temperature_02,
                      LGBAT_Data.BmsData.Data07.Balancing_R_Temperature,
                      LGBAT_Data.BmsData.Data07.PreCharge_R_Temperature,
                      LGBAT_Data.BmsData.Data07.CheckSum);
            break;

        // Data ID 0x08: FET Temperature and Software Version
        case LGBAT_DATA_ID_FET_STATUS:
            OS_printf("LGBAT I2C [0x08 %s]:"
                      " FET_Down_Temperature=%d FET_Up_Temperature=%d"
                      " CtrlCBStatus=%u Temperature_002_Reserved=%u SoftVersion=%u CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data08.FET_Down_Temperature,
                      LGBAT_Data.BmsData.Data08.FET_Up_Temperature,
                      LGBAT_Data.BmsData.Data08.CtrlCBStatus,
                      LGBAT_Data.BmsData.Data08.Temperature_002_Reserved,
                      LGBAT_Data.BmsData.Data08.SoftVersion,
                      LGBAT_Data.BmsData.Data08.CheckSum);
            break;

        // Data ID 0x09: BMS Status
        case LGBAT_DATA_ID_BMS_STATUS:
            OS_printf("LGBAT I2C [0x09 %s]:"
                      " BMS_Wakeup=%u WakeupHoldStatus=0x%02X"
                      " Wakeup_Hold_MCU=%u BMS_Wakeup_Signal=%u"
                      " VoltageDiag=%u CurrentDiag=%u TempDiag=%u FailureLevel=%u"
                      " DFET=%u CFET=%u CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data09.BMS_Wakeup,
                      LGBAT_Data.BmsData.Data09.WakeupHoldStatus,
                      LGBAT_0x09_GET_WAKEUP_HOLD_MCU(LGBAT_Data.BmsData.Data09.WakeupHoldStatus),
                      LGBAT_0x09_GET_BMS_WAKEUP_SIGNAL(LGBAT_Data.BmsData.Data09.WakeupHoldStatus),
                      LGBAT_0x09_GET_VOLTAGE_DIAG(LGBAT_Data.BmsData.Data09.VoltCurrDiag),
                      LGBAT_0x09_GET_CURRENT_DIAG(LGBAT_Data.BmsData.Data09.VoltCurrDiag),
                      LGBAT_0x09_GET_TEMP_DIAG(LGBAT_Data.BmsData.Data09.TempFailLevel),
                      LGBAT_0x09_GET_FAILURE_LEVEL(LGBAT_Data.BmsData.Data09.TempFailLevel),
                      LGBAT_0x09_GET_DFET(LGBAT_Data.BmsData.Data09.FETStatus),
                      LGBAT_0x09_GET_CFET(LGBAT_Data.BmsData.Data09.FETStatus),
                      LGBAT_Data.BmsData.Data09.CheckSum);
            break;

        // Data ID 0x0A: Failure Status
        case LGBAT_DATA_ID_FAIL_STATUS:
            OS_printf("LGBAT I2C [0x0A %s]:"
                      " FailStatus2=0x%02X"
                      " Timeout_Precharge=%u OverTemp_Precharge=%u OverTemp_CB=%u"
                      " AFE_CommLoss=%u AFE_Shutdown=%u"
                      " TempOpen_Cell=%u TempOpen_CB=%u TempOpen_FET=%u\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data0A.FailStatus2,
                      LGBAT_0x0A_GET_TIMEOUT_PRECHARGE(LGBAT_Data.BmsData.Data0A.FailStatus2),
                      LGBAT_0x0A_GET_OVERTEMP_PRECHARGE(LGBAT_Data.BmsData.Data0A.FailStatus2),
                      LGBAT_0x0A_GET_OVERTEMP_CB(LGBAT_Data.BmsData.Data0A.FailStatus2),
                      LGBAT_0x0A_GET_AFE_COMMLOSS(LGBAT_Data.BmsData.Data0A.FailStatus2),
                      LGBAT_0x0A_GET_AFE_SHUTDOWN(LGBAT_Data.BmsData.Data0A.FailStatus2),
                      LGBAT_0x0A_GET_TEMPOPEN_CELL(LGBAT_Data.BmsData.Data0A.FailStatus2),
                      LGBAT_0x0A_GET_TEMPOPEN_CB(LGBAT_Data.BmsData.Data0A.FailStatus2),
                      LGBAT_0x0A_GET_TEMPOPEN_FET(LGBAT_Data.BmsData.Data0A.FailStatus2));
            OS_printf("LGBAT I2C [0x0A %s]:"
                      " FailStatus3=0x%02X DFET_Fault=%u OverTemp_CDFET=%u CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data0A.FailStatus3,
                      LGBAT_0x0A_GET_DFET_FAULT(LGBAT_Data.BmsData.Data0A.FailStatus3),
                      LGBAT_0x0A_GET_OVERTEMP_CDFET(LGBAT_Data.BmsData.Data0A.FailStatus3),
                      LGBAT_Data.BmsData.Data0A.CheckSum);
            break;

        // Data ID 0x0B: Boost and MCU Voltages
        case LGBAT_DATA_ID_MCU_VOLTAGE:
            OS_printf("LGBAT I2C [0x0B %s]:"
                      " BOOST_Out_Voltage=%u MCU_B_Plus_Volt=%u"
                      " MCU_P_Plus_Volt=%u MCU_BMIC_REG_Out_Volt=%u CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data0B.BOOST_Out_Voltage,
                      LGBAT_Data.BmsData.Data0B.MCU_B_Plus_Volt,
                      LGBAT_Data.BmsData.Data0B.MCU_P_Plus_Volt,
                      LGBAT_Data.BmsData.Data0B.MCU_BMIC_REG_Out_Volt,
                      LGBAT_Data.BmsData.Data0B.CheckSum);
            break;

        // Data ID 0x0C: PCB Current and Boost Status
        case LGBAT_DATA_ID_DCDC_STATUS:
            OS_printf("LGBAT I2C [0x0C %s]:"
                      " BMS_Current=%d PCB_Temperature=%d"
                      " BOOST_Temperature=%d MCU_SBC_AMUX_Voltage=%u CheckSum=0x%02X\n",
                      LGBAT_GetDataIdName(DataID),
                      LGBAT_Data.BmsData.Data0C.BMS_Current,
                      LGBAT_Data.BmsData.Data0C.PCB_Temperature,
                      LGBAT_Data.BmsData.Data0C.BOOST_Temperature,
                      LGBAT_Data.BmsData.Data0C.MCU_SBC_AMUX_Voltage,
                      LGBAT_Data.BmsData.Data0C.CheckSum);
            break;

        default:
            break;
    }
}


// BMS I2C Data Parsing: copy raw big-endian bytes into structs.

static void LGBAT_ParseBmsData(uint8_t DataID, const uint8_t *Buf)
{
    switch (DataID)
    {
        case 0x01:
            LGBAT_Data.BmsData.Data01.ID                    = Buf[0];
            LGBAT_Data.BmsData.Data01.Pack_Voltage          = LGBAT_BE16(&Buf[1]);
            LGBAT_Data.BmsData.Data01.Pack_Current          = LGBAT_BE16S(&Buf[3]);
            LGBAT_Data.BmsData.Data01.Average_Time_To_Empty = LGBAT_BE16(&Buf[5]);
            LGBAT_Data.BmsData.Data01.Average_Time_To_Full  = LGBAT_BE16(&Buf[7]);
            LGBAT_Data.BmsData.Data01.CheckSum              = Buf[9];
            break;

        case 0x02:
            LGBAT_Data.BmsData.Data02.ID                    = Buf[0];
            LGBAT_Data.BmsData.Data02.Power_Supply_Status   = Buf[1];
            LGBAT_Data.BmsData.Data02.SOH                   = Buf[2];
            LGBAT_Data.BmsData.Data02.SOC                   = LGBAT_BE16(&Buf[3]);
            LGBAT_Data.BmsData.Data02.RC_Remaining_Capacity = LGBAT_BE16(&Buf[5]);
            LGBAT_Data.BmsData.Data02.AE_Available_Energy   = LGBAT_BE16(&Buf[7]);
            LGBAT_Data.BmsData.Data02.CheckSum              = Buf[9];

            // Warn if SOC is below the sleep threshold (10%)
            if (LGBAT_Data.BmsData.Data02.SOC < LGBAT_SOC_SLEEP_THRESHOLD_X100)
            {
                CFE_EVS_SendEvent(LGBAT_BMS_LOW_SOC_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "LGBAT: SOC low (%u/100 %%). BMS will enter Sleep.",
                                  LGBAT_Data.BmsData.Data02.SOC);
            }
            break;

        case 0x03:
            LGBAT_Data.BmsData.Data03.ID                                  = Buf[0];
            LGBAT_Data.BmsData.Data03.System_Max_Voltage                  = LGBAT_BE16(&Buf[1]);
            LGBAT_Data.BmsData.Data03.Power_Supply_Health                 = Buf[3];
            LGBAT_Data.BmsData.Data03.FETTestRequiredVoltage              = LGBAT_BE16(&Buf[4]);
            LGBAT_Data.BmsData.Data03.Battery_Information_Reserved[0]     = Buf[6];
            LGBAT_Data.BmsData.Data03.Battery_Information_Reserved[1]     = Buf[7];
            LGBAT_Data.BmsData.Data03.Battery_Information_Reserved[2]     = Buf[8];
            LGBAT_Data.BmsData.Data03.CheckSum                            = Buf[9];
            break;

        case 0x04:
            LGBAT_Data.BmsData.Data04.ID              = Buf[0];
            LGBAT_Data.BmsData.Data04.Percentage      = LGBAT_BE16(&Buf[1]);
            LGBAT_Data.BmsData.Data04.Design_Capacity = LGBAT_BE16(&Buf[3]);
            LGBAT_Data.BmsData.Data04.Capacity        = LGBAT_BE16(&Buf[5]);
            LGBAT_Data.BmsData.Data04.Charge          = LGBAT_BE16(&Buf[7]);
            LGBAT_Data.BmsData.Data04.CheckSum        = Buf[9];
            break;

        case 0x05:
            LGBAT_Data.BmsData.Data05.ID                   = Buf[0];
            LGBAT_Data.BmsData.Data05.Cell_Voltage_Max     = LGBAT_BE16(&Buf[1]);
            LGBAT_Data.BmsData.Data05.Cell_Voltage_Min     = LGBAT_BE16(&Buf[3]);
            LGBAT_Data.BmsData.Data05.Cell_Temperature_Max = LGBAT_BE16S(&Buf[5]);
            LGBAT_Data.BmsData.Data05.Cell_Temperature_Min = LGBAT_BE16S(&Buf[7]);
            LGBAT_Data.BmsData.Data05.CheckSum             = Buf[9];
            break;

        case 0x06:
            LGBAT_Data.BmsData.Data06.ID              = Buf[0];
            LGBAT_Data.BmsData.Data06.Cell_Voltage_01 = LGBAT_BE16(&Buf[1]);
            LGBAT_Data.BmsData.Data06.Cell_Voltage_02 = LGBAT_BE16(&Buf[3]);
            LGBAT_Data.BmsData.Data06.Reserved_01     = LGBAT_BE16(&Buf[5]);
            LGBAT_Data.BmsData.Data06.Reserved_02     = LGBAT_BE16(&Buf[7]);
            LGBAT_Data.BmsData.Data06.CheckSum        = Buf[9];
            break;

        case 0x07:
            // ICD buffer byte order: byte[1-2]=T02, byte[3-4]=T01
            LGBAT_Data.BmsData.Data07.ID                      = Buf[0];
            LGBAT_Data.BmsData.Data07.Cell_Temperature_02     = LGBAT_BE16S(&Buf[1]);
            LGBAT_Data.BmsData.Data07.Cell_Temperature_01     = LGBAT_BE16S(&Buf[3]);
            LGBAT_Data.BmsData.Data07.Balancing_R_Temperature = LGBAT_BE16S(&Buf[5]);
            LGBAT_Data.BmsData.Data07.PreCharge_R_Temperature = LGBAT_BE16S(&Buf[7]);
            LGBAT_Data.BmsData.Data07.CheckSum                = Buf[9];
            break;

        case 0x08:
            LGBAT_Data.BmsData.Data08.ID                       = Buf[0];
            LGBAT_Data.BmsData.Data08.FET_Down_Temperature     = LGBAT_BE16S(&Buf[1]);
            LGBAT_Data.BmsData.Data08.FET_Up_Temperature       = LGBAT_BE16S(&Buf[3]);
            LGBAT_Data.BmsData.Data08.CtrlCBStatus             = LGBAT_BE16(&Buf[5]);
            LGBAT_Data.BmsData.Data08.Temperature_002_Reserved = Buf[7];
            LGBAT_Data.BmsData.Data08.SoftVersion              = Buf[8];
            LGBAT_Data.BmsData.Data08.CheckSum                 = Buf[9];
            break;

        case 0x09:
            LGBAT_Data.BmsData.Data09.ID               = Buf[0];
            LGBAT_Data.BmsData.Data09.BMS_Wakeup       = Buf[1];
            LGBAT_Data.BmsData.Data09.WakeupHoldStatus = Buf[2];
            LGBAT_Data.BmsData.Data09.VoltCurrDiag     = Buf[3];
            LGBAT_Data.BmsData.Data09.TempFailLevel     = Buf[4];
            LGBAT_Data.BmsData.Data09.FETStatus         = Buf[5];
            LGBAT_Data.BmsData.Data09.Reserved[0]       = Buf[6];
            LGBAT_Data.BmsData.Data09.Reserved[1]       = Buf[7];
            LGBAT_Data.BmsData.Data09.Reserved[2]       = Buf[8];
            LGBAT_Data.BmsData.Data09.CheckSum          = Buf[9];
            break;

        case 0x0A:
            LGBAT_Data.BmsData.Data0A.ID          = Buf[0];
            LGBAT_Data.BmsData.Data0A.FailStatus2 = Buf[1];
            LGBAT_Data.BmsData.Data0A.FailStatus3 = Buf[2];
            memcpy(LGBAT_Data.BmsData.Data0A.Reserved, &Buf[3], 6);
            LGBAT_Data.BmsData.Data0A.CheckSum    = Buf[9];
            break;

        case 0x0B:
            LGBAT_Data.BmsData.Data0B.ID                    = Buf[0];
            LGBAT_Data.BmsData.Data0B.BOOST_Out_Voltage     = LGBAT_BE16(&Buf[1]);
            LGBAT_Data.BmsData.Data0B.MCU_B_Plus_Volt       = LGBAT_BE16(&Buf[3]);
            LGBAT_Data.BmsData.Data0B.MCU_P_Plus_Volt       = LGBAT_BE16(&Buf[5]);
            LGBAT_Data.BmsData.Data0B.MCU_BMIC_REG_Out_Volt = LGBAT_BE16(&Buf[7]);
            LGBAT_Data.BmsData.Data0B.CheckSum              = Buf[9];
            break;

        case 0x0C:
            LGBAT_Data.BmsData.Data0C.ID                   = Buf[0];
            LGBAT_Data.BmsData.Data0C.BMS_Current          = LGBAT_BE16S(&Buf[1]);
            LGBAT_Data.BmsData.Data0C.PCB_Temperature      = LGBAT_BE16S(&Buf[3]);
            LGBAT_Data.BmsData.Data0C.BOOST_Temperature    = LGBAT_BE16S(&Buf[5]);
            LGBAT_Data.BmsData.Data0C.MCU_SBC_AMUX_Voltage = LGBAT_BE16(&Buf[7]);
            LGBAT_Data.BmsData.Data0C.CheckSum             = Buf[9];
            break;

        default:
            break;
    }
}


// LGBAT_I2C_ReadBmsData

CFE_Status_t LGBAT_I2C_ReadBmsData(uint8_t DataID)
{
    uint8_t TxBuf[1];
    uint8_t RxBuf[LGBAT_BMS_BYTES_PER_ID];
    int32   Status;

    memset(RxBuf, 0, sizeof(RxBuf));

    CFE_SRL_IO_Handle_t *i2c = CFE_SRL_ApiGetHandle(LGBAT_I2C_HANDLE_INDEXER);
    if (i2c == NULL)
    {
        CFE_EVS_SendEvent(LGBAT_I2C_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: I2C2 handle NULL. Check I2C2 initialization.");
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    CFE_SRL_IO_Param_t param;
    memset(&param, 0, sizeof(param));
    TxBuf[0]      = DataID;
    param.Addr    = LGBAT_BMS_I2C_SLAVE_ADDR;
    param.TxData  = TxBuf;
    param.TxSize  = 1;
    param.RxData  = RxBuf;
    param.RxSize  = LGBAT_BMS_BYTES_PER_ID;
    param.Timeout = LGBAT_I2C_TIMEOUT_MS;

    Status = CFE_SRL_ApiRead(i2c, &param);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(LGBAT_I2C_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: I2C2 read fail. DataID=0x%02X RC=0x%08lX",
                          DataID, (unsigned long)Status);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    // Print raw receive buffer in ICD format
    LGBAT_PrintRawRxData(DataID, RxBuf, LGBAT_BMS_BYTES_PER_ID);

    // Verify that the first byte echoes the requested DataID
    if (RxBuf[0] != DataID)
    {
        CFE_EVS_SendEvent(LGBAT_I2C_READ_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: DataID mismatch. Expected=0x%02X Got=0x%02X",
                          DataID, RxBuf[0]);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    // Verify XOR checksum over bytes 0-8; result must equal byte 9
    if (!LGBAT_VerifyChecksum(RxBuf, LGBAT_BMS_BYTES_PER_ID))
    {
        CFE_EVS_SendEvent(LGBAT_I2C_CHECKSUM_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: XOR checksum error. DataID=0x%02X", DataID);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    // Parse raw bytes into the cached BMS data structure
    LGBAT_ParseBmsData(DataID, RxBuf);
    // Print parsed fields in ICD-aligned format
    LGBAT_PrintParsedData(DataID);

    return CFE_SUCCESS;
}


// LGBAT_CheckBmsHealth

void LGBAT_CheckBmsHealth(void)
{
    uint8_t FailLvl  = LGBAT_0x09_GET_FAILURE_LEVEL(LGBAT_Data.BmsData.Data09.TempFailLevel);
    uint8_t VoltDiag = LGBAT_0x09_GET_VOLTAGE_DIAG(LGBAT_Data.BmsData.Data09.VoltCurrDiag);
    uint8_t CurrDiag = LGBAT_0x09_GET_CURRENT_DIAG(LGBAT_Data.BmsData.Data09.VoltCurrDiag);
    uint8_t TempDiag = LGBAT_0x09_GET_TEMP_DIAG(LGBAT_Data.BmsData.Data09.TempFailLevel);
    uint8_t FS2      = LGBAT_Data.BmsData.Data0A.FailStatus2;
    uint8_t FS3      = LGBAT_Data.BmsData.Data0A.FailStatus3;

    bool sendCritical = false;

    if (FailLvl >= LGBAT_BMS_FAILURE_CRITICAL)
    {
        CFE_EVS_SendEvent(LGBAT_BMS_FAILURE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: CRITICAL! FailLvl=%u VoltDiag=%u CurrDiag=%u TempDiag=%u"
                          " FS2=0x%02X FS3=0x%02X",
                          FailLvl, VoltDiag, CurrDiag, TempDiag, FS2, FS3);
        sendCritical = true;
    }
    else if (FailLvl >= LGBAT_BMS_FAILURE_FAULT)
    {
        CFE_EVS_SendEvent(LGBAT_BMS_FAILURE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: FAULT! FailLvl=%u VoltDiag=%u CurrDiag=%u TempDiag=%u",
                          FailLvl, VoltDiag, CurrDiag, TempDiag);
        sendCritical = true;
    }
    else if (FailLvl >= LGBAT_BMS_FAILURE_WARNING)
    {
        CFE_EVS_SendEvent(LGBAT_BMS_FAILURE_ERR_EID, CFE_EVS_EventType_INFORMATION,
                          "LGBAT: WARNING FailLvl=%u VoltDiag=%u CurrDiag=%u TempDiag=%u",
                          FailLvl, VoltDiag, CurrDiag, TempDiag);
        sendCritical = true;
    }

    if (LGBAT_0x0A_ANY_FAIL2(FS2))
    {
        CFE_EVS_SendEvent(LGBAT_BMS_FAILURE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: FailStatus2=0x%02X", FS2);
        sendCritical = true;
    }

    if (LGBAT_0x0A_ANY_FAIL3(FS3))
    {
        CFE_EVS_SendEvent(LGBAT_BMS_FAILURE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: FailStatus3=0x%02X", FS3);
        sendCritical = true;
    }

    // Send a critical alert report when any fault or warning is detected
    if (sendCritical)
    {
        LGBAT_CriticalTlm_Payload_t *CP = &LGBAT_Data.CriticalTlm.Payload;
        CP->FailureLevel    = FailLvl;
        CP->VoltageDiag     = VoltDiag;
        CP->CurrentDiag     = CurrDiag;
        CP->TempDiag        = TempDiag;
        CP->FailStatus2_Raw = FS2;
        CP->FailStatus3_Raw = FS3;

        // Send via ReportTlm channel (RPT pattern)
        LGBAT_SendReport(LGBAT_CMD_MID, 0xFF, CFE_STATUS_EXTERNAL_RESOURCE_FAIL,
                         CP, sizeof(*CP));
        OS_printf("LGBAT: CriticalAlert sent. FailLvl=%u FS2=0x%02X FS3=0x%02X\n",
                  FailLvl, FS2, FS3);
    }
}


// Ground command handlers


// CC 0: No-operation command. Increments counter, sends RPT.
CFE_Status_t LGBAT_NoopCmd(const LGBAT_NoopCmd_t *Msg)
{
    (void)Msg;
    LGBAT_Data.CmdCounter++;
    OS_printf("LGBAT CMD [NOOP_CC]: NOOP received. CmdCounter=%u\n",
              LGBAT_Data.CmdCounter);
    CFE_EVS_SendEvent(LGBAT_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "LGBAT: NOOP. CmdCnt=%u", LGBAT_Data.CmdCounter);
    LGBAT_SendReport(LGBAT_CMD_MID, LGBAT_NOOP_CC, CFE_SUCCESS, NULL, 0);
    return CFE_SUCCESS;
}

// CC 1: Reset command and error counters to zero.
CFE_Status_t LGBAT_ResetCounterCmd(const LGBAT_ResetCounterCmd_t *Msg)
{
    (void)Msg;
    OS_printf("LGBAT CMD [RESET_COUNTER_CC]: CmdCounter=%u ErrCounter=%u -> 0\n",
              LGBAT_Data.CmdCounter, LGBAT_Data.ErrCounter);
    LGBAT_Data.CmdCounter = 0;
    LGBAT_Data.ErrCounter = 0;
    LGBAT_SendReport(LGBAT_CMD_MID, LGBAT_RESET_COUNTER_CC, CFE_SUCCESS, NULL, 0);
    return CFE_SUCCESS;
}

// CC 2 / SEND_BCN_MID handler: Compose and transmit beacon telemetry.
// Fills BcnTlm_Payload with BMS data from all 12 cached Data IDs.
CFE_Status_t LGBAT_SendBeaconCmd(void)
{
    LGBAT_BcnTlm_Payload_t *P = &LGBAT_Data.BcnTlm.Payload;

    OS_printf("LGBAT CMD [SEND_BCN_CC]: composing beacon. PowerApplied=%u\n",
              LGBAT_Data.PowerApplied);

    memset(P, 0, sizeof(*P));

    // App status fields
    P->CmdCounter    = LGBAT_Data.CmdCounter;
    P->CmdErrCounter = LGBAT_Data.ErrCounter;
    P->PowerApplied  = LGBAT_Data.PowerApplied ? 1u : 0u;

    if (LGBAT_Data.PowerApplied)
    {
        // Data ID 0x01: Pack Power and Time
        P->Pack_Voltage_mV       = LGBAT_Data.BmsData.Data01.Pack_Voltage;
        P->Pack_Current_mA       = LGBAT_Data.BmsData.Data01.Pack_Current;
        P->Avg_Time_To_Empty_sec = LGBAT_Data.BmsData.Data01.Average_Time_To_Empty;
        P->Avg_Time_To_Full_sec  = LGBAT_Data.BmsData.Data01.Average_Time_To_Full;

        // Data ID 0x02: Capacity and Status
        P->SOC_x100            = LGBAT_Data.BmsData.Data02.SOC;
        P->SOH_pct             = LGBAT_Data.BmsData.Data02.SOH;
        P->Power_Supply_Status = LGBAT_Data.BmsData.Data02.Power_Supply_Status;
        P->RC_Remaining_Cap    = LGBAT_Data.BmsData.Data02.RC_Remaining_Capacity;
        P->AE_Available_Energy = LGBAT_Data.BmsData.Data02.AE_Available_Energy;

        // Data ID 0x03: Battery Information
        P->System_Max_Voltage      = LGBAT_Data.BmsData.Data03.System_Max_Voltage;
        P->Power_Supply_Health     = LGBAT_Data.BmsData.Data03.Power_Supply_Health;
        P->FETTestRequiredVoltage  = LGBAT_Data.BmsData.Data03.FETTestRequiredVoltage;

        // Data ID 0x04: Charge State
        P->Charge_Percentage = LGBAT_Data.BmsData.Data04.Percentage;
        P->Design_Capacity   = LGBAT_Data.BmsData.Data04.Design_Capacity;
        P->Capacity          = LGBAT_Data.BmsData.Data04.Capacity;
        P->Charge            = LGBAT_Data.BmsData.Data04.Charge;

        // Data ID 0x05: Cell Voltage and Temperature Extremes
        P->Cell_Voltage_Max_mV = LGBAT_Data.BmsData.Data05.Cell_Voltage_Max;
        P->Cell_Voltage_Min_mV = LGBAT_Data.BmsData.Data05.Cell_Voltage_Min;
        P->Cell_Temp_Max_x10   = LGBAT_Data.BmsData.Data05.Cell_Temperature_Max;
        P->Cell_Temp_Min_x10   = LGBAT_Data.BmsData.Data05.Cell_Temperature_Min;

        // Data ID 0x06: Cell Voltages
        P->Cell_Voltage_01_mV = LGBAT_Data.BmsData.Data06.Cell_Voltage_01;
        P->Cell_Voltage_02_mV = LGBAT_Data.BmsData.Data06.Cell_Voltage_02;

        // Data ID 0x07: Cell Temperatures
        // Struct stores T02 first and T01 second (ICD buffer byte order preserved)
        P->Cell_Temp_01_x10     = LGBAT_Data.BmsData.Data07.Cell_Temperature_01;
        P->Cell_Temp_02_x10     = LGBAT_Data.BmsData.Data07.Cell_Temperature_02;
        P->Balancing_R_Temp_x10 = LGBAT_Data.BmsData.Data07.Balancing_R_Temperature;
        P->PreCharge_R_Temp_x10 = LGBAT_Data.BmsData.Data07.PreCharge_R_Temperature;

        // Data ID 0x08: FET Status and Software Version
        P->FET_Down_Temp_x10 = LGBAT_Data.BmsData.Data08.FET_Down_Temperature;
        P->FET_Up_Temp_x10   = LGBAT_Data.BmsData.Data08.FET_Up_Temperature;
        P->CtrlCBStatus      = LGBAT_Data.BmsData.Data08.CtrlCBStatus;
        P->SoftVersion       = LGBAT_Data.BmsData.Data08.SoftVersion;

        // Data ID 0x09: BMS Status
        // BMS_Wakeup is the raw HW status byte from the ICD; it is not an app flag.
        // The BMS wakes up automatically when 3.3V is applied.
        P->BMS_Wakeup        = LGBAT_Data.BmsData.Data09.BMS_Wakeup;
        P->BMS_Wakeup_Signal = LGBAT_0x09_GET_BMS_WAKEUP_SIGNAL(
                                   LGBAT_Data.BmsData.Data09.WakeupHoldStatus);
        P->Failure_Level     = LGBAT_0x09_GET_FAILURE_LEVEL(
                                   LGBAT_Data.BmsData.Data09.TempFailLevel);
        P->DFET_Status       = LGBAT_0x09_GET_DFET(LGBAT_Data.BmsData.Data09.FETStatus);
        P->CFET_Status       = LGBAT_0x09_GET_CFET(LGBAT_Data.BmsData.Data09.FETStatus);
        P->Voltage_Diag      = LGBAT_0x09_GET_VOLTAGE_DIAG(LGBAT_Data.BmsData.Data09.VoltCurrDiag);
        P->Current_Diag      = LGBAT_0x09_GET_CURRENT_DIAG(LGBAT_Data.BmsData.Data09.VoltCurrDiag);
        P->Temp_Diag         = LGBAT_0x09_GET_TEMP_DIAG(LGBAT_Data.BmsData.Data09.TempFailLevel);

        // Data ID 0x0A: Failure Status
        P->FailStatus2_Raw = LGBAT_Data.BmsData.Data0A.FailStatus2;
        P->FailStatus3_Raw = LGBAT_Data.BmsData.Data0A.FailStatus3;

        // Data ID 0x0B: Boost and MCU Voltages
        P->BOOST_Out_Voltage_mV     = LGBAT_Data.BmsData.Data0B.BOOST_Out_Voltage;
        P->MCU_B_Plus_Volt_mV       = LGBAT_Data.BmsData.Data0B.MCU_B_Plus_Volt;
        P->MCU_P_Plus_Volt_mV       = LGBAT_Data.BmsData.Data0B.MCU_P_Plus_Volt;
        P->MCU_BMIC_REG_Out_Volt_mV = LGBAT_Data.BmsData.Data0B.MCU_BMIC_REG_Out_Volt;

        // Data ID 0x0C: PCB Current and Boost Status
        P->BMS_Current_mA          = LGBAT_Data.BmsData.Data0C.BMS_Current;
        P->PCB_Temperature_x10     = LGBAT_Data.BmsData.Data0C.PCB_Temperature;
        P->BOOST_Temperature_x10   = LGBAT_Data.BmsData.Data0C.BOOST_Temperature;
        P->MCU_SBC_AMUX_Voltage_mV = LGBAT_Data.BmsData.Data0C.MCU_SBC_AMUX_Voltage;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(LGBAT_Data.BcnTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(LGBAT_Data.BcnTlm.TelemetryHeader), true);

    OS_printf("LGBAT CMD [SEND_BCN_CC]: beacon transmitted."
              " PowerApplied=%u Pack_Voltage_mV=%u SOC_x100=%u\n",
              P->PowerApplied, P->Pack_Voltage_mV, P->SOC_x100);

    // Send RPT only when called from the ground command CC path
    LGBAT_SendReport(LGBAT_CMD_MID, LGBAT_SEND_BCN_CC, CFE_SUCCESS, NULL, 0);
    return CFE_SUCCESS;
}

// CC 3: Read one BMS Data ID via I2C2.
CFE_Status_t LGBAT_RequestDataCmd(const LGBAT_RequestDataCmd_t *Msg)
{
    uint8_t DataID = Msg->DataID;

    if (DataID < LGBAT_BMS_DATA_ID_MIN || DataID > LGBAT_BMS_DATA_ID_MAX)
    {
        CFE_EVS_SendEvent(LGBAT_CC_ERR_EID, CFE_EVS_EventType_ERROR,
                          "LGBAT: Invalid DataID=0x%02X (valid range: 0x01-0x0C)", DataID);
        LGBAT_Data.ErrCounter++;
        LGBAT_SendReport(LGBAT_CMD_MID, LGBAT_REQUEST_DATA_CC,
                         CFE_STATUS_BAD_COMMAND_CODE, NULL, 0);
        return CFE_STATUS_BAD_COMMAND_CODE;
    }

    OS_printf("LGBAT CMD [REQUEST_DATA_CC]: DataID=0x%02X (%s)\n",
              DataID, LGBAT_GetDataIdName(DataID));

    LGBAT_Data.CmdCounter++;
    CFE_Status_t Status = LGBAT_I2C_ReadBmsData(DataID);
    if (Status != CFE_SUCCESS) LGBAT_Data.ErrCounter++;

    OS_printf("LGBAT CMD [REQUEST_DATA_CC]: DataID=0x%02X completed with Status=0x%08lX\n",
              DataID, (unsigned long)Status);

    LGBAT_SendReport(LGBAT_CMD_MID, LGBAT_REQUEST_DATA_CC, Status, NULL, 0);
    return Status;
}

// CC 4: Read all 12 Data IDs (0x01-0x0C) sequentially, then send HK telemetry.
// Skips I2C reads if 3.3V power is not applied.
CFE_Status_t LGBAT_RequestAllDataCmd(const LGBAT_RequestAllDataCmd_t *Msg)
{
    (void)Msg;

    OS_printf("LGBAT CMD [REQUEST_ALL_DATA_CC]: read all DataID 0x%02X through 0x%02X.\n",
              LGBAT_BMS_DATA_ID_MIN, LGBAT_BMS_DATA_ID_MAX);

    if (!LGBAT_Data.PowerApplied)
    {
        OS_printf("LGBAT CMD [REQUEST_ALL_DATA_CC]: skipped. PowerApplied=0\n");
        LGBAT_SendReport(LGBAT_CMD_MID, LGBAT_REQUEST_ALL_DATA_CC,
                         CFE_STATUS_NOT_IMPLEMENTED, NULL, 0);
        return CFE_SUCCESS;
    }

    LGBAT_Data.CmdCounter++;

    CFE_Status_t LastStatus = CFE_SUCCESS;
    for (uint8_t id = LGBAT_BMS_DATA_ID_MIN; id <= LGBAT_BMS_DATA_ID_MAX; id++)
    {
        OS_printf("LGBAT CMD [REQUEST_ALL_DATA_CC]: reading DataID=0x%02X (%s)\n",
                  id, LGBAT_GetDataIdName(id));
        CFE_Status_t s = LGBAT_I2C_ReadBmsData(id);
        if (s != CFE_SUCCESS)
        {
            LGBAT_Data.ErrCounter++;
            LastStatus = s;
        }
    }

    // Build and send HK telemetry with a summary of the BMS state
    LGBAT_HkTlm_Payload_t *HK = &LGBAT_Data.HkTlm.Payload;
    memset(HK, 0, sizeof(*HK));
    HK->CmdCounter         = LGBAT_Data.CmdCounter;
    HK->CmdErrCounter      = LGBAT_Data.ErrCounter;
    HK->PowerApplied       = LGBAT_Data.PowerApplied ? 1u : 0u;
    HK->MissionActive      = LGBAT_Data.MissionActive ? 1u : 0u;
    HK->FirstCommDone      = LGBAT_Data.FirstCommSuccess ? 1u : 0u;
    HK->Pack_Voltage_mV    = LGBAT_Data.BmsData.Data01.Pack_Voltage;
    HK->Pack_Current_mA    = LGBAT_Data.BmsData.Data01.Pack_Current;
    HK->SOC_x100           = LGBAT_Data.BmsData.Data02.SOC;
    HK->SOH_pct            = LGBAT_Data.BmsData.Data02.SOH;
    HK->Power_Supply_Status = LGBAT_Data.BmsData.Data02.Power_Supply_Status;
    HK->Failure_Level      = LGBAT_0x09_GET_FAILURE_LEVEL(
                                 LGBAT_Data.BmsData.Data09.TempFailLevel);
    HK->BMS_Wakeup         = LGBAT_Data.BmsData.Data09.BMS_Wakeup;
    HK->FailStatus2_Raw    = LGBAT_Data.BmsData.Data0A.FailStatus2;
    HK->FailStatus3_Raw    = LGBAT_Data.BmsData.Data0A.FailStatus3;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(LGBAT_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(LGBAT_Data.HkTlm.TelemetryHeader), true);
    OS_printf("LGBAT CMD [REQUEST_ALL_DATA_CC]: HK TLM transmitted.\n");

    // Run health check after every full poll cycle
    LGBAT_CheckBmsHealth();

    // Record the first successful full cycle
    if (LastStatus == CFE_SUCCESS && !LGBAT_Data.FirstCommSuccess)
    {
        LGBAT_Data.FirstCommSuccess = true;
        CFE_EVS_SendEvent(LGBAT_MISSION_START_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "LGBAT: First full BMS I2C cycle SUCCESS. Mission Criteria met.");
    }

    OS_printf("LGBAT CMD [REQUEST_ALL_DATA_CC]: completed. Status=0x%08lX\n",
              (unsigned long)LastStatus);

    LGBAT_SendReport(LGBAT_CMD_MID, LGBAT_REQUEST_ALL_DATA_CC, LastStatus, NULL, 0);
    return LastStatus;
}

// CC 5: Turn 3.3V power on or off.
// The BMS wakes up automatically when 3.3V is applied (no extra wakeup command needed).
CFE_Status_t LGBAT_SetPowerCmd(const LGBAT_SetPowerCmd_t *Msg)
{
    LGBAT_Data.CmdCounter++;
    OS_printf("LGBAT CMD [SET_POWER_CC]: PowerOn=%u\n", Msg->PowerOn);

    if (Msg->PowerOn)
    {
        LGBAT_Data.PowerApplied = true;
        CFE_EVS_SendEvent(LGBAT_BMS_POWER_ON_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "LGBAT: 3.3V ON. BMS will wake up automatically.");
        OS_printf("LGBAT: 3.3V ON. BMS wakeup is automatic.\n");
    }
    else
    {
        LGBAT_Data.PowerApplied = false;
        CFE_EVS_SendEvent(LGBAT_BMS_POWER_OFF_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "LGBAT: 3.3V OFF. BMS entering sleep.");
        OS_printf("LGBAT: 3.3V OFF. BMS entering sleep.\n");
    }

    LGBAT_SendReport(LGBAT_CMD_MID, LGBAT_SET_POWER_CC, CFE_SUCCESS, NULL, 0);
    return CFE_SUCCESS;
}

// CC 6: Reset cached BMS data and clear power applied flag.
CFE_Status_t LGBAT_ResetBmsCmd(const LGBAT_ResetBmsCmd_t *Msg)
{
    (void)Msg;
    LGBAT_Data.CmdCounter++;
    OS_printf("LGBAT CMD [RESET_BMS_CC]: clearing cached BMS data and power state.\n");
    LGBAT_Data.PowerApplied     = false;
    LGBAT_Data.FirstCommSuccess = false;
    memset(&LGBAT_Data.BmsData, 0, sizeof(LGBAT_Data.BmsData));
    LGBAT_SendReport(LGBAT_CMD_MID, LGBAT_RESET_BMS_CC, CFE_SUCCESS, NULL, 0);
    return CFE_SUCCESS;
}
