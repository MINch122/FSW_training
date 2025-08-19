#ifndef _UANT_H_
#define _UANT_H_

#include <stdint.h>

typedef enum {
/*  Command                                 Code      Param   Response 	*/
	UANT_CMD_RESET                    	  = 0xAA, /*  None    None		*/
	UANT_CMD_ARM_ANTENNA_SYSTEM       	  = 0xAD, /*  None    None		*/
	UANT_CMD_DISARM                   	  = 0xAC, /*  None    None 		*/
	UANT_CMD_DEPLOY_ANT1         		  = 0xA1, /*  u8      None 		*/
	UANT_CMD_DEPLOY_ANT2         		  = 0xA2, /*  u8      None 		*/
	UANT_CMD_DEPLOY_ANT3         		  = 0xA3, /*  u8      None 		*/
	UANT_CMD_DEPLOY_ANT4         		  = 0xA4, /*  u8      None 		*/
	UANT_CMD_AUTOMATED_DEPLOY		  	  = 0xA5, /*  u8      None 		*/
	UANT_CMD_DEPLOY_ANT1_OVERRIDE 		  = 0xBA, /*  u8      None 		*/
	UANT_CMD_DEPLOY_ANT2_OVERRIDE 		  = 0xBB, /*  u8      None 		*/
	UANT_CMD_DEPLOY_ANT3_OVERRIDE 		  = 0xBC, /*  u8      None 		*/
	UANT_CMD_DEPLOY_ANT4_OVERRIDE 		  = 0xBD, /*  u8      None 		*/
	UANT_CMD_CANCEL_DEPLOYMENT_ACTIVATION = 0xA9, /*  None 	  None 		*/
	UANT_CMD_MEASURE_SYSTEM_TEMPERATURE   = 0xC0, /*  None	  u8[2]		*/
	UANT_CMD_REPORT_DEPLOYMENT_STATUS  	  = 0xC3, /*  None    u8[2]		*/
	UANT_CMD_REPORT_ANT1_ACTIVATION_CNT   = 0xB0, /*  None 	  u8	  	*/
	UANT_CMD_REPORT_ANT2_ACTIVATION_CNT   = 0xB1, /*  None 	  u8	  	*/
	UANT_CMD_REPORT_ANT3_ACTIVATION_CNT   = 0xB2, /*  None 	  u8	  	*/
	UANT_CMD_REPORT_ANT4_ACTIVATION_CNT   = 0xB3, /*  None 	  u8	  	*/
	UANT_CMD_REPORT_ANT1_ACTIVATION_TIME  = 0xB4, /*  None 	  u8[2]		*/
	UANT_CMD_REPORT_ANT2_ACTIVATION_TIME  = 0xB5, /*  None 	  u8[2]		*/
	UANT_CMD_REPORT_ANT3_ACTIVATION_TIME  = 0xB6, /*  None 	  u8[2]		*/
	UANT_CMD_REPORT_ANT4_ACTIVATION_TIME  = 0xB7, /*  None 	  u8[2]		*/
} uant_cmd_code_t;  /// 레지스터 번호임 


int ISIS_UANT_SendCmd(uint8_t reg);

int ISIS_UANT_SendCmdWithResponse(uint8_t reg,
								  uint8_t respLen,
								  void* resp);

int ISIS_UANT_SendCmdWithParam(uint8_t reg,
							   uint8_t param);

#define ISIS_UANT_Reset() \
		ISIS_UANT_SendCmd(UANT_CMD_RESET)

#define ISIS_UANT_ArmAntennaSystems() \
		ISIS_UANT_SendCmd(UANT_CMD_ARM_ANTENNA_SYSTEM)

#define ISIS_UANT_Disarm() \
		ISIS_UANT_SendCmd(UANT_CMD_DISARM)

#define ISIS_UANT_DeployAntenna1(burnTime) \
		ISIS_UANT_SendCmdWithParam(UANT_CMD_DEPLOY_ANT1, burnTime)

#define ISIS_UANT_DeployAntenna2(burnTime) \
		ISIS_UANT_SendCmdWithParam(UANT_CMD_DEPLOY_ANT2, burnTime)

#define ISIS_UANT_DeployAntenna3(burnTime) \
		ISIS_UANT_SendCmdWithParam(UANT_CMD_DEPLOY_ANT3, burnTime)

#define ISIS_UANT_DeployAntenna4(burnTime) \
		ISIS_UANT_SendCmdWithParam(UANT_CMD_DEPLOY_ANT4, burnTime)

#define ISIS_UANT_AutomatedSequentialDeployment(burnTime) \
		ISIS_UANT_SendCmdWithParam(UANT_CMD_AUTOMATED_DEPLOY, burnTime)

#define ISIS_UANT_DeployAntenna1WithOverride(burnTime) \
		ISIS_UANT_SendCmdWithParam(UANT_CMD_DEPLOY_ANT1_OVERRIDE, burnTime)

#define ISIS_UANT_DeployAntenna2WithOverride(burnTime) \
		ISIS_UANT_SendCmdWithParam(UANT_CMD_DEPLOY_ANT2_OVERRIDE, burnTime)

#define ISIS_UANT_DeployAntenna3WithOverride(burnTime) \
		ISIS_UANT_SendCmdWithParam(UANT_CMD_DEPLOY_ANT3_OVERRIDE, burnTime)

#define ISIS_UANT_DeployAntenna4WithOverride(burnTime) \
		ISIS_UANT_SendCmdWithParam(UANT_CMD_DEPLOY_ANT4_OVERRIDE, burnTime)

#define ISIS_UANT_CancelDeploymentSystemActivation() \
		ISIS_UANT_SendCmd(UANT_CMD_CANCEL_DEPLOYMENT_ACTIVATION)

int ISIS_UANT_MeasureAntennaSystemTemperature(uint16_t *rawVoltage);

int ISIS_UANT_ReportDeploymentStatus(uint16_t *DeploymentStatus);

int ISIS_UANT_ReportAntennaActivationCount(uint8_t AntennaNumber,
										   uint8_t* ActivationCount);

int ISIS_UANT_ReportAntennaActivationTime(uint8_t AntennaNumber,
										   uint16_t* ActivationTime);

#endif