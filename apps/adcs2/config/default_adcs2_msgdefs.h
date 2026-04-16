/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   Specification for the ADCS command and telemetry
 *   message constant definitions.
 *
 *  For ADCS this is only the function/command code definitions
 */
#ifndef ADCS2_MSGDEFS_H
#define ADCS2_MSGDEFS_H

#include "common_types.h"
#include "adcs2_fcncodes.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                   << ADCS BCN, HK, AOD Structures >>                      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// typedef struct
// {
//     /** Combined Power State
//      *  | 7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
//      *  +--------------------------------------+
//      *  |Rsv|RWL0|RWL1|RWL2|MAG0|GYRO|FSS0|HSS0|
//      *  +--------------------------------------+
//      */
//     uint8 PowerState; // ID 183

//     uint8 ControlMode; // ID 185

//     float GYR0CalibratedRateXComponent;
//     float GYR0CalibratedRateYComponent;
//     float GYR0CalibratedRateZComponent; // ID 207, 12bytes

// } __attribute__((packed)) ADCS2_BcnTlm_Payload_t; /* Total 14 bytes */

// typedef struct
// {
//     uint16 MAG0MCUCurrent; // 2 bytes, ID 167

//     int16  FSS0MCUTemperature;
//     uint16 FSS0MCUCurrent;
//     uint16 FSS0MCUVoltage;
//     uint8  FSS0CAMSRAMOvercurrent; // combined, 7 bytes, ID 168

//     float MTQ1PositiveCurrentAverage;
//     float MTQ1NegativeCurrentAverage;
//     float MTQ2PositiveCurrentAverage;
//     float MTQ2NegativeCurrentAverage;
//     float MTQ3PositiveCurrentAverage;
//     float MTQ3NegativeCurrentAverage;
//     uint8 MTQPolarity; // MTQ 1,2,3 (combined), 25 bytes, ID 169

//     int16  HSS0MCUTemperature;
//     uint16 HSS0MCUCurrent;
//     uint16 HSS0MCUVoltage; // 6 bytes, ID 217

//     int16  RWL0MCUTemperature;
//     uint16 RWL0MCUCurrent;
//     uint16 RWL0BatteryVoltage;
//     uint16 RWL0BatteryCurrent;
//     int16  RWL1MCUTemperature;
//     uint16 RWL1MCUCurrent;
//     uint16 RWL1BatteryVoltage;
//     uint16 RWL1BatteryCurrent;
//     int16  RWL2MCUTemperature;
//     uint16 RWL2MCUCurrent;
//     uint16 RWL2BatteryVoltage;
//     uint16 RWL2BatteryCurrent; // 24 bytes, ID 218

//     int16 MTQ0OpenLoopOnTimeCommand;
//     int16 MTQ1OpenLoopOnTimeCommand;
//     int16 MTQ2OpenLoopOnTimeCommand; // 6bytes, ID 182

//     uint8 FSS0CaptureResult;
//     uint8 FSS0DetectionResult; // 2bytes, ID 170

//     uint8 HSS0CaptureResult;
//     uint8 HSS0DetectionResult; // 2bytes, ID 179

//     uint32 bytesReceived; // 4bytes, ID 158

//     float  mtq0Mmax;  /**< MTQ0 maximum dipole moment  (measurment unit is [A.m^2] */
//     float  mtq1Mmax;  /**< MTQ1 maximum dipole moment  (measurment unit is [A.m^2] */
//     float  mtq2Mmax;  /**< MTQ2 maximum dipole moment  (measurment unit is [A.m^2] */
//     uint16 onTimeMax; /**< Maximum magnetorquer on-time  (measurment unit is [ms]) */
//     float  mtqFfac;
//         /**< LPF factor for magnetorquer commands. Set to zero for no filtering  (valid range is between 0  and 1 ) */ // 18 bytes, ID 198

//     uint8 css0Raw;
//     uint8 css1Raw;
//     uint8 css2Raw;
//     uint8 css3Raw;
//     uint8 css4Raw;
//     uint8 css5Raw;
//     uint8 css6Raw;
//     uint8 css7Raw;
//     uint8 css8Raw;
//     uint8 css9Raw;
//     uint8  rawCssIsValid:1; // 11bytes, ID 203

//     int16 cssCalVecX;
//     int16 cssCalVecY;
//     int16 cssCalVecZ;
//     uint8  calCssIsValid:1; // 7 bytes, ID 206
// } ADCS2_HkTlm_Payload_t;  // 107 Bytes


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                     ADCS Set Cmd Payload Structures                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* FORMAT OF STRUCT
typedef struct { // ID

}__attribute__((packed)) ADCS2_***Cmd_Payload_t;
*/

typedef struct
{                                  // ID 2
    uint32 CurrentUnixseconds;     // Current Unix time s. (Unit of measure is [s])
    uint32 CurrentUnixNanoseconds; // Current Unix time ns. (Unit of measure is [ns])
} __attribute__((packed)) ADCS2_CurrentUnixTimeCmd_Payload_t;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      ADCS Get Cmd Payload Structures                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* FORMAT OF STRUCT
typedef struct { // ID

}__attribute__((packed)) ADCS2_***Tlm_Payload_t;
*/

typedef struct
{                                  // ID 133
    uint32 CurrentUnixseconds;     // Current Unix time s. (Unit of measure is [s])
    uint32 CurrentUnixNanoseconds; // Current Unix time ns. (Unit of measure is [ns])
} __attribute__((packed)) ADCS2_CurrentUnixTimeTlm_Payload_t;

typedef struct
{ // ID 172
    uint32	TimeSeconds;
    uint32	TimeNanoSeconds;
    int16	CommandedRollAngle;		// [deg * 100]
    int16	CommandedPitchAngle;	// [deg * 100]
    int16	CommandedYawAngle;		// [deg * 100]
    int16	ErrorRollAngle;			// [deg * 100]
    int16	ErrorPitchAngle;		// [deg * 100]
    int16	ErrorYawAngle;			// [deg * 100]
	float	RefGndorGEOTargetLat;	// [deg]
	float	RefGndorGEOTargetLon;	// [deg]
	float	RefGndorGEOTargetAlt;	// [km]
    int16	ControlErrorQ0;			// [* 10000]
    int16	ControlErrorQ1;			// [* 10000]
    int16	ControlErrorQ2;			// [* 10000]

	float	RWL0SpeedCommand;		// [RPM]
	float	RWL1SpeedCommand;		// [RPM]
	float	RWL2SpeedCommand;		// [RPM]
	float	RWL3SpeedCommand;		// [RPM]
	float	RWL0MomentumCommand;	// [Nms]
	float	RWL1MomentumCommand;	// [Nms]
	float	RWL2MomentumCommand;	// [Nms]
	float	RWL3MomentumCommand;	// [Nms]
	float	RWL0TorqueCommand;		// [Nm]
	float	RWL1TorqueCommand;		// [Nm]
	float	RWL2TorqueCommand;		// [Nm]
	float	RWL3TorqueCommand;		// [Nm]

	int16	MTQ0DipoleMomCommand;	// [Am^2 * 1000]
	int16	MTQ1DipoleMomCommand;	// [Am^2 * 1000]
	int16	MTQ2DipoleMomCommand;	// [Am^2 * 1000]
	float	MTQ0TorqueCommand;		// [Nm]
	float	MTQ1TorqueCommand;		// [Nm]
	float	MTQ2TorqueCommand;		// [Nm]
	int16	MTQ0OnTimeCommand;		// [ms]
	int16	MTQ1OnTimeCommand;		// [ms]
	int16	MTQ2OnTimeCommand;		// [ms]

	uint16	ControlTimeout;			// [sec]
    uint8	ActiveContMode;

	uint8	RWL0ErrorFlag:1;
	uint8	RWL1ErrorFlag:1;
	uint8	RWL2ErrorFlag:1;
	uint8	RWL3ErrorFlag:1;
	uint8	RWL0ActiveFlag:1;
	uint8	RWL1ActiveFlag:1;
	uint8	RWL2ActiveFlag:1;
	uint8	RWL3ActiveFlag:1;

} __attribute__((packed)) ADCS2_ControllerTlm_Payload_t;


typedef struct
{ // ID 177
    uint32	TimeSeconds;
    uint32	TimeNanoSeconds;

    int16	MAG0CalVecX;
    int16	MAG0CalVecY;
    int16	MAG0CalVecZ;

    int16	MAG1CalVecX;
    int16	MAG1CalVecY;
    int16	MAG1CalVecZ;

	uint8	MAG0ValidFlag:1;
	uint8	MAG1ValidFlag:1;
	uint8	MAG0BestFlag:1;
	uint8	MAG1BestFlag:1;

} __attribute__((packed)) ADCS2_CalibratedMAGSensorTlm_Payload_t;

typedef struct
{ // ID 180
    uint32 TimeSeconds;
    uint32 TimeNanoSeconds;
    int16 MAG0RawVecX;
    int16 MAG0RawVecY;
    int16 MAG0RawVecZ;
    int16 MAG1RawVecX;
    int16 MAG1RawVecY;
    int16 MAG1RawVecZ;
    uint8 MAG0ValidFlag:1;
    uint8 MAG1ValidFlag:1;
} __attribute__((packed)) ADCS2_RawMAGSensorTlm_Paylaod_t;

typedef struct
{ // ID 185
    uint8  ControlMode;
    uint16 ControlTimeout;
} __attribute__((packed)) ADCS2_ControlModeTlm_Payload_t;

typedef struct
{ // ID 203
    uint32	TimeSeconds;
    uint32	TimeNanoSeconds;
    uint8	CSS0;
    uint8	CSS1;
    uint8	CSS2;
    uint8	CSS3;
    uint8	CSS4;
    uint8	CSS5;
    uint8	CSS6;
    uint8	CSS7;
    uint8	CSS8;
    uint8	CSS9;
    uint8	CSSValidFlag:1;
} __attribute__((packed)) ADCS2_RawCSSSensorTlm_Payload_t;

typedef struct
{ // ID 204
    uint32 TimeSeconds;
    uint32 TimeNanoSeconds;
    float GYR0RawRateX;
    float GYR0RawRateY;
    float GYR0RawRateZ;
    float GYR1RawRateX;
    float GYR1RawRateY;
    float GYR1RawRateZ;
    uint8 GYR0ValidFlag:1;
    uint8 GYR1ValidFlag:1;
} __attribute__((packed)) ADCS2_RawGYRSensorTlm_Paylaod_t;

typedef struct
{ // ID 207
    uint32 TimeSeconds;
    uint32 TimeNanoSeconds;
    float  GYR0CalibratedRateX;
    float  GYR0CalibratedRateY;
    float  GYR0CalibratedRateZ;
    float  GYR1CalibratedRateX;
    float  GYR1CalibratedRateY;
    float  GYR1CalibratedRateZ;
    float  ExtGYR0CalibratedRateX;
    float  ExtGYR0CalibratedRateY;
    float  ExtGYR0CalibratedRateZ;
    float  ExtGYR1CalibratedRateX;
    float  ExtGYR1CalibratedRateY;
    float  ExtGYR1CalibratedRateZ;
    uint8   GYR0ValidFlag:1;
    uint8   GYR1ValidFlag:1;
    uint8   EXTGYR0ValidFlag:1;
    uint8   EXTGYR1ValidFlag:1;
} __attribute__((packed)) ADCS2_CalibratedGYRSensorTlm_Payload_t;


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                ADCS Set/Get Cmd Common Payload Structures                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* FORMAT OF STRUCT
typedef struct { // ID

}__attribute__((packed)) ADCS2_***_Cmn_Payload_t;
*/

typedef struct
{ // ID 42 & 150
    uint8  ControlMode;
    uint8  MainEstimatorMode;
    uint8  BackupEstimatorMode;
    uint16 ControlTimeout;
} __attribute__((packed)) ADCS2_ControlEstimationMode_Cmn_Payload_t;

typedef struct
{ // ID 56 & 183
    uint8 RWL0;
    uint8 RWL1;
    uint8 RWL2;
    uint8 RWL3;
    uint8 MAG0;
    uint8 MAG1;
    uint8 GYR0;
    uint8 GYR1;
    uint8 FSS0;
    uint8 FSS1;
    uint8 FSS2;
    uint8 FSS3;
    uint8 HSS0;
    uint8 HSS1;
    uint8 STR0;
    uint8 STR1;
    uint8 ExtSensor0;
    uint8 ExtSensor1;
    uint8 ExtGYR0;
    uint8 ExtGYR1;
} __attribute__((packed)) ADCS2_PowerState_Cmn_Payload_t;

typedef struct
{ // ID 65 & 193
    /* Stack & Actuators (ENUM, 1B each) */
    uint8 StackX_mounting; // StackX mounting (Table 43)
    uint8 StackY_mounting; // StackY mounting (Table 43)
    uint8 StackZ_mounting; // StackZ mounting (Table 43)
    uint8 MTQ0_mounting;   // MTQ0 mounting (Table 43)
    uint8 MTQ1_mounting;   // MTQ1 mounting (Table 43)
    uint8 MTQ2_mounting;   // MTQ2 mounting (Table 43)
    uint8 Wheel0_mounting; // Wheel0 mounting (Table 43)
    uint8 Wheel1_mounting; // Wheel1 mounting (Table 43)
    uint8 Wheel2_mounting; // Wheel2 mounting (Table 43)
    uint8 Wheel3_mounting; // Wheel3 mounting (Table 43)

    /* Pyramid RWL angles (INT16; deg = raw/100.0) */
    int16 PyramidRWL_alpha; // alpha angle
    int16 PyramidRWL_beta;  // beta angle
    int16 PyramidRWL_gamma; // gamma angle

    /* CSS mounting (ENUM, 1B each) */
    uint8 CSS0_mounting;
    uint8 CSS1_mounting;
    uint8 CSS2_mounting;
    uint8 CSS3_mounting;
    uint8 CSS4_mounting;
    uint8 CSS5_mounting;
    uint8 CSS6_mounting;
    uint8 CSS7_mounting;
    uint8 CSS8_mounting;
    uint8 CSS9_mounting;

    /* FSS0..3 angles (INT16; deg = raw/100.0) */
    int16 FSS0_alpha;
    int16 FSS0_beta;
    int16 FSS0_gamma;
    int16 FSS1_alpha;
    int16 FSS1_beta;
    int16 FSS1_gamma;
    int16 FSS2_alpha;
    int16 FSS2_beta;
    int16 FSS2_gamma;
    int16 FSS3_alpha;
    int16 FSS3_beta;
    int16 FSS3_gamma;

    /* HSS0..1 angles (INT16; deg = raw/100.0) */
    int16 HSS0_alpha;
    int16 HSS0_beta;
    int16 HSS0_gamma;
    int16 HSS1_alpha;
    int16 HSS1_beta;
    int16 HSS1_gamma;

    /* MAG0..1 angles (INT16; deg = raw/100.0) */
    int16 MAG0_alpha;
    int16 MAG0_beta;
    int16 MAG0_gamma;
    int16 MAG1_alpha;
    int16 MAG1_beta;
    int16 MAG1_gamma;

    /* STR0..1 angles (INT16; deg = raw/100.0) */
    int16 STR0_alpha;
    int16 STR0_beta;
    int16 STR0_gamma;
    int16 STR1_alpha;
    int16 STR1_beta;
    int16 STR1_gamma;

    /* External sensor 0/1 angles (INT16; deg = raw/100.0) */
    int16 ExtSensor0_alpha;
    int16 ExtSensor0_beta;
    int16 ExtSensor0_gamma;
    int16 ExtSensor1_alpha;
    int16 ExtSensor1_beta;
    int16 ExtSensor1_gamma;

    /* External gyro axis mounting (ENUM, 1B each; Table 43) */
    uint8 ExtGyro0_axis1_mounting;
    uint8 ExtGyro0_axis2_mounting;
    uint8 ExtGyro0_axis3_mounting;
    uint8 ExtGyro1_axis1_mounting;
    uint8 ExtGyro1_axis2_mounting;
    uint8 ExtGyro1_axis3_mounting;
} __attribute__((packed)) ADCS2_MountingConfig_Cmn_Payload_t;

typedef struct
{ // ID 173 & 210
    uint32 TimeSecond;
    uint32 TimeNanoSecond;
    int16  EstRoll;
    int16  EstPitch;
    int16  EstYaw;
    int16  EstORCQ0;
    int16  EstORCQ1;
    int16  EstORCQ2;
    int16  EstORCQ3;
    int16  EstGYRbiasX;
    int16  EstGYRbiasY;
    int16  EstGYRbiasZ;
    int16  EstORCBodyRateX;
    int16  EstORCBodyRateY;
    int16  EstORCBodyRateZ;
    int16  EstIRCBodyRateX;
    int16  EstIRCBodyRateY;
    int16  EstIRCBodyRateZ;
    float  EstGyroscTrqX;
    float  EstGyroscTrqY;
    float  EstGyroscTrqZ;
    int16  InnvVecX;
    int16  InnvVecY;
    int16  InnvVecZ;
    int16  StdEstRateX;
    int16  StdEstRateY;
    int16  StdEstRateZ;
    int16  StdEstQ0;
    int16  StdEstQ1;
    int16  StdEstQ2;
    uint8  ActiveEstMode;
} __attribute__((packed)) ADCS2_Estimator_Cmn_Payload_t;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*               ADCS Commissioning Result Payload Structures                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct
{	// COMM TLM AMOUNT FLAG

	uint8	flag_tlmtype;
	uint8	flag_estmode;
	uint8	flag_contmode;
	
} __attribute__((packed)) ADCS2_COMM_FLAG_Payload_t;

typedef struct
{	// COMM 01 - Compact

	// Simple Header
	uint16	sync_word;
	
	// ID 210
    uint32	MainEst_TimeSecond;
    uint32	MainEst_TimeNanoSecond;	
    int16	MainEst_EstORCBodyRateX;
    int16	MainEst_EstORCBodyRateY;
    int16	MainEst_EstORCBodyRateZ;
    int16	MainEst_EstIRCBodyRateX;
    int16	MainEst_EstIRCBodyRateY;
    int16	MainEst_EstIRCBodyRateZ;
    uint8	MainEst_ActiveEstMode;
	
	// ID 173
    uint32	BackupEst_TimeSecond;
    uint32	BackupEst_TimeNanoSecond;	
    int16	BackupEst_EstORCBodyRateX;
    int16	BackupEst_EstORCBodyRateY;
    int16	BackupEst_EstORCBodyRateZ;
    int16	BackupEst_EstIRCBodyRateX;
    int16	BackupEst_EstIRCBodyRateY;
    int16	BackupEst_EstIRCBodyRateZ;
    uint8	BackupEst_ActiveEstMode;
	
	// ID 204
    uint32	GYR0_TimeSeconds;
    uint32	GYR0_TimeNanoSeconds;
    float	GYR0RawRateX;
    float	GYR0RawRateY;
    float	GYR0RawRateZ;
	
	// ID 180
    uint32	MAG0_TimeSeconds;
    uint32	MAG0_TimeNanoSeconds;
    int16	MAG0RawVecX;
    int16	MAG0RawVecY;
    int16	MAG0RawVecZ;

	uint8	GYR0ValidFlag:1;
    uint8	MAG0ValidFlag:1;

} __attribute__((packed)) ADCS2_COMM_01_COMP_Payload_t; /* Total 76.25 bytes */

typedef struct
{	// COMM 01 - Full

	// Simple Header
	uint16	sync_word;

	// ID 210
    ADCS2_Estimator_Cmn_Payload_t	MainEst;
	
	// ID 173
    ADCS2_Estimator_Cmn_Payload_t	BackupEst;
	
	// ID 204
	ADCS2_RawGYRSensorTlm_Paylaod_t	GYR0;
	
	// ID 180
    ADCS2_RawMAGSensorTlm_Paylaod_t MAG0;

} __attribute__((packed)) ADCS2_COMM_01_FULL_Payload_t; /* Total 194.5 bytes */


typedef struct
{	// COMM 02 - Compact

	// Simple Header
	uint16	sync_word;
	
	// ID 210
    uint32	MainEst_TimeSecond;
    uint32	MainEst_TimeNanoSecond;
    int16	MainEst_EstIRCBodyRateX;
    int16	MainEst_EstIRCBodyRateY;
    int16	MainEst_EstIRCBodyRateZ;
    uint8	MainEst_ActiveEstMode;
	
	// ID 204
    uint32	GYR0_TimeSeconds;
    uint32	GYR0_TimeNanoSeconds;
    float	GYR0RawRateX;
    float	GYR0RawRateY;
    float	GYR0RawRateZ;
	
	// ID 180
    uint32	MAG0_TimeSeconds;
    uint32	MAG0_TimeNanoSeconds;
    int16	MAG0RawVecX;
    int16	MAG0RawVecY;
    int16	MAG0RawVecZ;

	// ID 203
	uint32	CSS_TimeSeconds;
    uint32	CSS_TimeNanoSeconds;
    uint8	CSS0;
    uint8	CSS1;
    uint8	CSS2;
    uint8	CSS3;
    uint8	CSS4;
    uint8	CSS5;
    uint8	CSS6;
    uint8	CSS7;
    uint8	CSS8;
    uint8	CSS9;

	// ID 172
    uint32	Cont_TimeSeconds;
    uint32	Cont_TimeNanoSeconds;
	uint16	ControlTimeout;			// [sec]
    uint8	ActiveContMode;


	uint8	GYR0ValidFlag:1;
    uint8	MAG0ValidFlag:1;
    uint8	CSSValidFlag:1;

} __attribute__((packed)) ADCS2_COMM_02_COMP_Payload_t; /* Total 87.375 bytes */

typedef struct
{	// COMM 02 - Full

	// Simple Header
	uint16	sync_word;

	// ID 210
    ADCS2_Estimator_Cmn_Payload_t	MainEst;
		
	// ID 204
	ADCS2_RawGYRSensorTlm_Paylaod_t	GYR0;
	
	// ID 180
    ADCS2_RawMAGSensorTlm_Paylaod_t MAG0;

	// ID 203
	ADCS2_RawCSSSensorTlm_Payload_t	CSS;

	// ID 172
	ADCS2_ControllerTlm_Payload_t	Cont;

} __attribute__((packed)) ADCS2_COMM_02_FULL_Payload_t; /* Total 173.625 bytes */

typedef struct
{	// COMM 03 - Compact

	// Simple Header
	uint16	sync_word;
	
	// ID 210
    uint32	MainEst_TimeSecond;
    uint32	MainEst_TimeNanoSecond;
    int16	MainEst_EstIRCBodyRateX;
    int16	MainEst_EstIRCBodyRateY;
    int16	MainEst_EstIRCBodyRateZ;
    uint8	MainEst_ActiveEstMode;
	
	// ID 177
    uint32	MAG0_TimeSeconds;
    uint32	MAG0_TimeNanoSeconds;
    int16	MAG0CalVecX;
    int16	MAG0CalVecY;
    int16	MAG0CalVecZ;

    uint8	MAG0ValidFlag:1;

} __attribute__((packed)) ADCS2_COMM_03_COMP_Payload_t; /* Total 29.25 bytes */

typedef struct
{	// COMM 03 - Full

	// Simple Header
	uint16	sync_word;

	// ID 210
    ADCS2_Estimator_Cmn_Payload_t	MainEst;

	// ID 177
    ADCS2_CalibratedMAGSensorTlm_Payload_t MAG0;

} __attribute__((packed)) ADCS2_COMM_03_FULL_Payload_t; /* Total 85.25 bytes */



/*************************************
 * CubeADCS Event Entry
 *************************************/
// typedef struct
// {
//     uint32_t Counter;
//     uint32_t UpTime;
//     uint32_t UnixTime;
//     uint16_t MilliSec;
//     struct __attribute__((packed))
//     {
//         uint16_t EventType : 9;
//         uint8_t  EventSource : 5;
//         uint8_t  EventClass : 2;
//     } Identifier;
//     uint8_t EventData[8];
// } __attribute__((packed)) ADCS2_EventEntry_t;

/*************************************
 * CubeADCS Frame Something
 *************************************/
// typedef struct
// {
//     uint32_t Counter;
//     uint32_t UpTime;
//     uint32_t UnixTime;
//     uint16_t MilliSec;
//     struct __attribute__((packed))
//     {
//         uint16_t EventType : 9;
//         uint8_t  EventSource : 5;
//         uint8_t  EventClass : 2;
//     } Identifier;
//     uint8_t EventData[8];
// } __attribute__((packed)) ADCS2_Frame_t;

#endif