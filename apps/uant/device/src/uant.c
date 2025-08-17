#include <stdint.h>
#include <stdbool.h>
#include "../inc/uant/uant.h"
#include "uant_app_mission_cfg.h"
#include "uant_app.h"

int ISIS_UANT_SendCmd(uint8_t cc) {
    CFE_SRL_IO_Param_t params = {0};
    uint8_t reg = cc;  // 전송할 레지스터 주소

    params.TxData  = &reg;    // Tx 버퍼: 레지스터
    params.TxSize  = 1;       // 버퍼 크기: 1바이트
    params.Timeout = 10;
    params.Addr    = UANT_APP_I2C_ADDR;   // 타임아웃: 10ms

    // I2C 쓰기 수행
    int32 status = CFE_SRL_ApiWrite(UANT_APP_Data.Handle, &params);
    return status;
}


int ISIS_UANT_SendCmdWithParam(uint8_t cc, uint8_t param) {
    CFE_SRL_IO_Param_t params = {0};
    uint8_t buf[2] = { cc, param };  // 레지스터 + 파라미터

    params.TxData  = buf;         // Tx 버퍼 설정
    params.TxSize  = sizeof(buf); // 버퍼 크기: 2바이트
    params.Timeout = 10;          // 타임아웃: 10ms
    params.Addr    = UANT_APP_I2C_ADDR; 
    // I2C 쓰기 수행
    int32 status = CFE_SRL_ApiWrite(UANT_APP_Data.Handle, &params);
    return status;
}



int ISIS_UANT_SendCmdWithResponse(uint8_t cc, uint8_t respLen, void* resp) {
    CFE_SRL_IO_Param_t params = {0};
    uint8_t reg = cc;  // 전송할 레지스터 주소

    params.TxData  = &reg;        // Tx: 레지스터 전송
    params.TxSize  = 1;           // 크기: 1바이트
    params.RxData  = (uint8_t*)resp;  // Rx: 응답 버퍼
    params.RxSize  = respLen;        // 읽을 바이트 수
    params.Timeout = 100;            // 타임아웃: 100ms
    params.Addr    = UANT_APP_I2C_ADDR; 
    // I2C 읽기 수행
    int32 status = CFE_SRL_ApiRead(UANT_APP_Data.Handle, &params);
    return status;
}


int ISIS_UANT_MeasureAntennaSystemTemperature(uint16_t *rawVoltage) {
    // 응답 버퍼 크기: uint16_t (2바이트)
    return ISIS_UANT_SendCmdWithResponse(
        UANT_CMD_MEASURE_SYSTEM_TEMPERATURE,
        sizeof(*rawVoltage),
        rawVoltage
    );
}



int ISIS_UANT_ReportDeploymentStatus(uint16_t *DeploymentStatus) {
    return ISIS_UANT_SendCmdWithResponse(
        UANT_CMD_REPORT_DEPLOYMENT_STATUS,
        sizeof(*DeploymentStatus),
        DeploymentStatus
    );
}


int ISIS_UANT_ReportAntennaActivationCount(uint8_t AntennaNumber,
                                           uint8_t* ActivationCount) {
    uint8_t reg;
    // 안테나 번호에 따른 레지스터 매핑
    switch (AntennaNumber) {
        case 1: reg = UANT_CMD_REPORT_ANT1_ACTIVATION_CNT; break;
        case 2: reg = UANT_CMD_REPORT_ANT2_ACTIVATION_CNT; break;
        case 3: reg = UANT_CMD_REPORT_ANT3_ACTIVATION_CNT; break;
        case 4: reg = UANT_CMD_REPORT_ANT4_ACTIVATION_CNT; break;
        default:
            return CFE_SRL_BAD_ARGUMENT;
    }
    return ISIS_UANT_SendCmdWithResponse(
        reg,
        sizeof(*ActivationCount),
        ActivationCount
    );
}


int ISIS_UANT_ReportAntennaActivationTime(uint8_t AntennaNumber,
                                          uint16_t* ActivationTime) {

    uint8_t reg;
    // 안테나 번호에 따른 레지스터 매핑
    switch (AntennaNumber) {
        case 1: reg = UANT_CMD_REPORT_ANT1_ACTIVATION_TIME; break;
        case 2: reg = UANT_CMD_REPORT_ANT2_ACTIVATION_TIME; break;
        case 3: reg = UANT_CMD_REPORT_ANT3_ACTIVATION_TIME; break;
        case 4: reg = UANT_CMD_REPORT_ANT4_ACTIVATION_TIME; break;
        default:
            return CFE_SRL_BAD_ARGUMENT; 
    }
    
    return ISIS_UANT_SendCmdWithResponse(
        reg,
        sizeof(*ActivationTime),
        ActivationTime
    );
}
