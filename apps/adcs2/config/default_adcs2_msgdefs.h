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
typedef struct
{
    /** Combined Power State
     *  | 7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
     *  +--------------------------------------+
     *  |Rsv|RWL0|RWL1|RWL2|MAG0|GYRO|FSS0|HSS0|
     *  +--------------------------------------+
     */
    uint8 PowerState; // ID 183

    uint8 ControlMode; // ID 185

    float GYR0CalibratedRateXComponent;
    float GYR0CalibratedRateYComponent;
    float GYR0CalibratedRateZComponent; // ID 207, 12bytes

    uint8 CSS[6]; // ID 203

} __attribute__((packed)) ADCS2_BcnTlm_Payload_t; /* Total 20 bytes */

typedef struct
{
    uint32 CurrentUnixseconds;
    uint32 CurrentUnixNanoseconds;

    uint8 ControlMode;
    uint8 MainEstimatorMode;
    uint8 BackupEstimatorMode;
    uint16 ControlTimeout;

    uint8 PowerState;

    int16 MAG0RawVecX;
    int16 MAG0RawVecY;
    int16 MAG0RawVecZ;
    uint8 MAG0ValidFlag:1;

    uint8 CSS0;
    uint8 CSS1;
    uint8 CSS2;
    uint8 CSS3;
    uint8 CSS4;
    uint8 CSS5;
    uint8 CSSValidFlag:1;

    float GYR0RawRateX;
    float GYR0RawRateY;
    float GYR0RawRateZ;
    uint8 GYR0ValidFlag:1;

    float RWL0MeasSpeed;
    float RWL1MeasSpeed;
    float RWL2MeasSpeed;
    float RWL3MeasSpeed;

    int16 CSSCalUnitVecX;
    int16 CSSCalUnitVecY;
    int16 CSSCalUnitVecZ;
    uint8 CalCSSValidFlag:1;

    float GYR0CalibratedRateX;
    float GYR0CalibratedRateY;
    float GYR0CalibratedRateZ;
    uint8 CalGYR0ValidFlag:1;

    int16 MainEstRoll;
    int16 MainEstPitch;
    int16 MainEstYaw;
    int16 MainEstIRCBodyRateX;
    int16 MainEstIRCBodyRateY;
    int16 MainEstIRCBodyRateZ;
    uint8 MainActiveEstMode;

    int16 BackupEstRoll;
    int16 BackupEstPitch;
    int16 BackupEstYaw;
    uint8 BackupActiveEstMode;

    uint16 ControllerTimeout;
    uint8 ActiveContMode;
} __attribute__((packed)) ADCS2_HkTlm_Payload_t;


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

typedef struct
{ // ID 54
    float Roll;
    float Pitch;
    float Yaw;
} __attribute__((packed)) ADCS2_ReferenceRPYvaluesCmd_Payload_t;

typedef struct
{ // ID 76
    float cmdHx;
    float cmdHy;
    float cmdHz;
} __attribute__((packed)) ADCS2_OpenLoopCmdHxyzRWCmd_Payload_t;


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
{ // ID 170
    uint32 TimeSecond;
    uint32 TimeNanoSecond;
    int16  FSS0AlphaAngle;
    int16  FSS0BetaAngle;
    uint8  FSS0CaptureResult;
    uint8  FSS0DetectionResult;
    int16  FSS1AlphaAngle;
    int16  FSS1BetaAngle;
    uint8  FSS1CaptureResult;
    uint8  FSS1DetectionResult;
    int16  FSS2AlphaAngle;
    int16  FSS2BetaAngle;
    uint8  FSS2CaptureResult;
    uint8  FSS2DetectionResult;
    int16  FSS3AlphaAngle;
    int16  FSS3BetaAngle;
    uint8  FSS3CaptureResult;
    uint8  FSS3DetectionResult;
    uint8  ValidResult0:1; // FSS0
    uint8  ValidResult1:1; // FSS1
    uint8  ValidResult2:1; // FSS2
    uint8  ValidResult3:1; // FSS3
} __attribute__((packed)) ADCS2_RawCubeSenseSunTlm_Payload_t;

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
    uint8   FmcStage;

} __attribute__((packed)) ADCS2_ControllerTlm_Payload_t;

typedef struct
{ // ID 174
    uint32	TimeSeconds;
	uint32	TimeNanoSeconds;
	int32	posXeci;
	int32	posYeci;
	int32	posZeci;
	int16	velXeci;
	int16	velYeci;
	int16	velZeci;
	int16	LatGeoD;
	int16	LatGeoC;
	int16	Lon;
	uint32	Alt;
	int32	posXeciTarg;
	int32	posYeciTarg;
	int32	posZeciTarg;
	int16	MagORCX;
	int16	MagORCY;
	int16	MagORCZ;
	int16	SunORCX;
	int16	SunORCY;
	int16	SunORCZ;
	int16	SunBetaAng;
	int16	HorzAngXoYo;
	int16	HorzAngEastXo;
	int16	HorzAngEastYi;
	int32	GndTargORCX;
	int32	GndTargORCY;
	int32	GndTargORCZ;
	uint16	ASGP4BatchCnt;
	uint16	ASGP4PosDelta;
	uint8	ActiveOrbitMode:2;
	uint8	ASGP4PosErr:1;
	uint8	flagEclipse:1;
	uint8	SrcOrb:2;

} __attribute__((packed)) ADCS2_ModelsTlm_Payload_t;

typedef struct
{ // ID 176
    uint32	TimeSeconds;
    uint32	TimeNanoSeconds;

    int16	HSS0CalVecX;
    int16	HSS0CalVecY;
    int16	HSS0CalVecZ;

    int16	HSS1CalVecX;
    int16	HSS1CalVecY;
    int16	HSS1CalVecZ;

	uint8	HSS0ValidFlag:1;
	uint8	HSS1ValidFlag:1;

} __attribute__((packed)) ADCS2_CalibratedHSSSensorTlm_Payload_t;

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
{ // ID 178
    uint32	TimeSeconds;
    uint32	TimeNanoSeconds;

    int16	FSS0CalVecX;
    int16	FSS0CalVecY;
    int16	FSS0CalVecZ;

    int16	FSS1CalVecX;
    int16	FSS1CalVecY;
    int16	FSS1CalVecZ;

    int16	FSS2CalVecX;
    int16	FSS2CalVecY;
    int16	FSS2CalVecZ;

    int16	FSS3CalVecX;
    int16	FSS3CalVecY;
    int16	FSS3CalVecZ;

	uint8	FSS0ValidFlag:1;
	uint8	FSS1ValidFlag:1;
	uint8	FSS2ValidFlag:1;
	uint8	FSS3ValidFlag:1;
	uint8	FSS0BestFlag:1;
	uint8	FSS1BestFlag:1;
	uint8	FSS2BestFlag:1;
	uint8	FSS3BestFlag:1;

} __attribute__((packed)) ADCS2_CalibratedFSSSensorTlm_Payload_t;

typedef struct
{ // ID 179
    uint32 TimeSecond;
    uint32 TimeNanoSecond;
    int16  HSS0RawElevationAngle;
    int16  HSS0RawRotationAngle;
    uint8  HSS0CaptureResult;
    uint8  HSS0DetectionResult;
    int16  HSS1RawElevationAngle;
    int16  HSS1RawRotationAngle;
    uint8  HSS1CaptureResult;
    uint8  HSS1DetectionResult;
    uint8  HSS0ValidFlag:1; // HSS0
    uint8  HSS1ValidFlag:1; // HSS1
} __attribute__((packed)) ADCS2_RawCubeSenseEarthTlm_Payload_t;

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
} __attribute__((packed)) ADCS2_RawGYRSensorTlm_Payload_t;

typedef struct
{ // ID 205
    uint32 TimeSeconds;
    uint32 TimeNanoSeconds;
    float RWL0MeasSpeed;
    float RWL1MeasSpeed;
    float RWL2MeasSpeed;
    float RWL3MeasSpeed;
    uint8 RWL0ValidFlag:1;
    uint8 RWL1ValidFlag:1;
    uint8 RWL2ValidFlag:1;
    uint8 RWL3ValidFlag:1;
} __attribute__((packed)) ADCS2_RawRWLSensorTlm_Payload_t;

typedef struct
{ // ID 206
    uint32 TimeSeconds;
    uint32 TimeNanoSeconds;
    int16 CSSCalUnitVecX;
    int16 CSSCalUnitVecY;
    int16 CSSCalUnitVecZ;
    uint8 CSSValidFlag:1;
} __attribute__((packed)) ADCS2_CalibratedCSSSensorTlm_Payload_t;

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

typedef struct
{ // ID 209
    uint32	TimeSeconds;
    uint32	TimeNanoSeconds;
    float	WhlSBCTrqX;
    float	WhlSBCTrqY;
    float	WhlSBCTrqZ;
    float	WhlSBCMomX;
    float	WhlSBCMomY;
    float	WhlSBCMomZ;
    uint8   RWLValidFlag:1;
} __attribute__((packed)) ADCS2_CalibratedRWLSensorTlm_Payload_t;


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
{ // ID 51 & 162
    uint8 OrbitMode;
} __attribute__((packed)) ADCS2_OrbitMode_Cmn_Payload_t;

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
{ // ID 67 & 195
    uint8 DefaultMainEstimatorMode;
    uint8 DefaultBackupEstimatorMode;
    float MAGMeasurementNoise;
    float CSSMeasurementNoise;
    float FSSMeasurementNoise;
    float HSSMeasurementNoise;
    float STRMeasurementNoise;
    float MMTRKFSystemNoise;
    float EKFSystemNoise;
    float NutationEpsilonCorrection;
    float NutationPsiCorrection;

    uint8 UseFSSinEKF : 1;
    uint8 UseCSSinEKF : 1;
    uint8 UseHSSinEKF : 1;
    uint8 UseSTRinEKF : 1;
    uint8 TriadVector1 : 4;
    uint8 TriadVector2 : 4;

} __attribute__((packed)) ADCS2_EstimatorConfig_Cmn_Payload_t;

typedef struct
{ // ID 68 & 196
    double Epoch;
    double Inclination;
    double RAAN;
    double Eccentricity;
    double AOP;
    double MeanAnomaly;
    double MeanMotion;
    double B_StarDrag;
} __attribute__((packed)) ADCS2_SatOrbitParamConfig_Cmn_Payload_t;

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
	ADCS2_RawGYRSensorTlm_Payload_t	GYR0;
	
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
	ADCS2_RawGYRSensorTlm_Payload_t	GYR0;
	
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


typedef struct
{	// COMM 04 - Compact

	// Simple Header
	uint16	sync_word;

	// ID 210
    ADCS2_Estimator_Cmn_Payload_t	MainEst;

	// ID 203
    ADCS2_RawCSSSensorTlm_Payload_t	RawCSS;

	// ID 206
	ADCS2_CalibratedCSSSensorTlm_Payload_t CalCSS;

	// ID 174
	ADCS2_ModelsTlm_Payload_t	Models;

} __attribute__((packed)) ADCS2_COMM_04_COMP_Payload_t; /* Total 190 bytes */

typedef struct
{	// COMM 04 - Full

	// Simple Header
	uint16	sync_word;

	// ID 210
    ADCS2_Estimator_Cmn_Payload_t	MainEst;
	
	// ID 173
    ADCS2_Estimator_Cmn_Payload_t	BackupEst;

	// ID 203
    ADCS2_RawCSSSensorTlm_Payload_t	RawCSS;

	// ID 206
	ADCS2_CalibratedCSSSensorTlm_Payload_t CalCSS;

	// ID 174
	ADCS2_ModelsTlm_Payload_t	Models;

} __attribute__((packed)) ADCS2_COMM_04_FULL_Payload_t;

typedef struct
{	// COMM 05

	// Simple Header
	uint16	sync_word;

	// ID 210
    ADCS2_Estimator_Cmn_Payload_t	MainEst;

	// ID 203
    ADCS2_RawCSSSensorTlm_Payload_t	RawCSS;

	// ID 206
	ADCS2_CalibratedCSSSensorTlm_Payload_t CalCSS;

	// ID 170
	ADCS2_RawCubeSenseSunTlm_Payload_t	RawFSS;

	// ID 178
	ADCS2_CalibratedFSSSensorTlm_Payload_t	CalFSS;

} __attribute__((packed)) ADCS2_COMM_05_Payload_t; /* Total 190 bytes */

typedef struct
{	// COMM 06

	// Simple Header
	uint16	sync_word;

	// ID 210
    ADCS2_Estimator_Cmn_Payload_t	MainEst;

	// ID 205
    ADCS2_RawRWLSensorTlm_Payload_t	RawRWL;

	// ID 209
	ADCS2_CalibratedRWLSensorTlm_Payload_t CalRWL;

} __attribute__((packed)) ADCS2_COMM_06_Payload_t; /* Total ?? bytes */

typedef struct
{	// COMM 07

	// Simple Header
	uint16	sync_word;

	// ID 210
    ADCS2_Estimator_Cmn_Payload_t	MainEst;

	// ID 205
    ADCS2_RawRWLSensorTlm_Payload_t	RawRWL;

	// ID 209
	ADCS2_CalibratedRWLSensorTlm_Payload_t CalRWL;

	// ID 178
	ADCS2_CalibratedFSSSensorTlm_Payload_t	CalFSS;

} __attribute__((packed)) ADCS2_COMM_07_Payload_t; /* Total ?? bytes */

typedef struct
{	// COMM 08

	// Simple Header
	uint16	sync_word;

	// ID 210
    ADCS2_Estimator_Cmn_Payload_t	MainEst;

	// ID 179
    ADCS2_RawCubeSenseEarthTlm_Payload_t	RawHSS;

	// ID 176
	ADCS2_CalibratedHSSSensorTlm_Payload_t	CalHSS;


} __attribute__((packed)) ADCS2_COMM_08_Payload_t; /* Total ?? bytes */

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

typedef struct
{
    uint8 TransportType;
} __attribute__((packed)) ADCS2_InterfaceTransportCmd_Payload_t;

#endif
