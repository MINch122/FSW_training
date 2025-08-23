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
#ifndef ADCS_MSGDEFS_H
#define ADCS_MSGDEFS_H

#include "common_types.h"
#include "adcs_fcncodes.h"

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
    uint8 PowerState; // 183

    uint8 ControlMode; // 185

    float GYR0CalibratedRateXComponent;
    float GYR0CalibratedRateYComponent;
    float GYR0CalibratedRateZComponent; // ID 207, 12bytes

}__attribute__((packed)) ADCS_BcnTlm_Payload_t; /* Total 14 bytes */


typedef struct
{
    uint16 MAG0MCUCurrent; // 2 bytes, ID 167
    
    int16 FSS0MCUTemperature;
    uint16 FSS0MCUCurrent;
    uint16 FSS0MCUVoltage;
    uint8 FSS0CAMSRAMOvercurrent; // combined, 7 bytes, ID 168

    float MTQ1PositiveCurrentAverage;
    float MTQ1NegativeCurrentAverage;
    float MTQ2PositiveCurrentAverage;
    float MTQ2NegativeCurrentAverage;
    float MTQ3PositiveCurrentAverage;
    float MTQ3NegativeCurrentAverage;
    uint8 MTQPolarity; // MTQ 1,2,3 (combined), 25 bytes, ID 169

    int16 HSS0MCUTemperature;
    uint16 HSS0MCUCurrent;
    uint16 HSS0MCUVoltage; // 6 bytes, ID 217

    int16 RWL0MCUTemperature;
    uint16 RWL0MCUCurrent;
    uint16 RWL0BatteryVoltage;
    uint16 RWL0BatteryCurrent;
    int16 RWL1MCUTemperature;
    uint16 RWL1MCUCurrent;
    uint16 RWL1BatteryVoltage;
    uint16 RWL1BatteryCurrent;
    int16 RWL2MCUTemperature;
    uint16 RWL2MCUCurrent;
    uint16 RWL2BatteryVoltage;
    uint16 RWL2BatteryCurrent; //24 bytes, ID 218

    int16 MTQ0OpenLoopOnTimeCommand;
    int16 MTQ1OpenLoopOnTimeCommand;
    int16 MTQ2OpenLoopOnTimeCommand; //6bytes, ID 182

    uint8 FSS0CaptureResult;
    uint8 FSS0DetectionResult; // 2bytes, ID 170
    
    uint8 HSS0CaptureResult;
    uint8 HSS0DetectionResult; // 2bytes, ID 179

    uint32 bytesReceived; //4bytes, ID 158

    float mtq0Mmax;     /**< MTQ0 maximum dipole moment  (measurment unit is [A.m^2] */
    float mtq1Mmax;     /**< MTQ1 maximum dipole moment  (measurment unit is [A.m^2] */
    float mtq2Mmax;     /**< MTQ2 maximum dipole moment  (measurment unit is [A.m^2] */
    uint16 onTimeMax;    /**< Maximum magnetorquer on-time  (measurment unit is [ms]) */
    float mtqFfac;      /**< LPF factor for magnetorquer commands. Set to zero for no filtering  (valid range is between 0  and 1 ) */ // 18 bytes, ID 198

    uint8 css0Raw;
    uint8 css1Raw;
    uint8 css2Raw;
    uint8 css3Raw;
    uint8 css4Raw;
    uint8 css5Raw;
    uint8 css6Raw;
    uint8 css7Raw;
    uint8 css8Raw;
    uint8 css9Raw;
    bool rawCssIsValid; // 11bytes, ID 203

    int16 cssCalVecX;
    int16 cssCalVecY;
    int16 cssCalVecZ;
    bool calCssIsValid; // 7 bytes, ID 206
} ADCS_HkTlm_Payload_t; // 107 Bytes

typedef struct
{
    uint32 unixTimeSeconds;
    uint32 unixTimeNanoSeconds; //8bytes, ID 133

    float RWL0MeasuredSpeed;
    float RWL1MeasuredSpeed;
    float RWL2MeasuredSpeed; //12 bytes, ID 205

    int16 FSS0AlphaAngle;
    int16 FSS0BetaAngle; //4bytes, ID 170

    int16 SatelliteGeocentricLatitude;
    int16 SatelliteLongitude;
    uint32 SatelliteAltitude;
    int16 SunORCModelXComponent;
    int16 SunORCModelYComponent;
    int16 SunORCModelZComponent;
    int16 SunBetaAngleWithOrbitPlane; // 16 bytes, ID 174

    int16 hss0CalVecX;
    int16 hss0CalVecY;
    int16 hss0CalVecZ; //6 bytes, ID 176

    int16 mag0CalVecX;
    int16 mag0CalVecY;
    int16 mag0CalVecZ; //6 bytes, ID 177

    int16 fss0CalVecX;
    int16 fss0CalVecY;
    int16 fss0CalVecZ; //6 bytes, ID 178

    int16 hss0RawElevationAngle;
    int16 hss0RawRotationAngle; // 4 bytes, ID 179

    int16 MAG0RawVectorXComponent;
    int16 MAG0RawVectorYComponent;
    int16 MAG0RawVectorZComponent; // 6 bytes, ID 180

    float GYR0RawRateXComponent;
    float GYR0RawRateYComponent;
    float GYR0RawRateZComponent; // 12 bytes, ID 204

    uint32 TimeIntegerSeconds;
    int16 EstimatedRollAngle;
    int16 EstimatedPitchAngle;
    int16 EstimatedYawAngle;
    int16 EstimatedORCQuaternionQ0;
    int16 EstimatedORCQuaternionQ1;
    int16 EstimatedORCQuaternionQ2;
    int16 EstimatedORCQuaternionQ3;
    int16 EstimatedBodyRateORCXComponent;
    int16 EstimatedBodyRateORCYComponent;
    int16 EstimatedBodyRateORCZComponent;
    int16 EstimatedBodyRateIRCXComponent;
    int16 EstimatedBodyRateIRCYComponent;
    int16 EstimatedBodyRateIRCZComponent;
    int16 InnovationVectorXComponent;
    int16 InnovationVectorYComponent;
    int16 InnovationVectorZComponent;
    int16 StdDevOfEstimatedRateXComponent;
    int16 StdDevOfEstimatedRateYComponent;
    int16 StdDevOfEstimatedRateZComponent;
    int16 StdDevOfEstimatedQuaternionQ0Component;
    int16 StdDevOfEstimatedQuaternionQ1Component;
    int16 StdDevOfEstimatedQuaternionQ2Component; //48 bytes, ID 210

    float ixx;                 /**< Moment of inertia Ixx  (measurment unit is [kg.m^2]) */
    float iyy;                 /**< Moment of inertia Iyy  (measurment unit is [kg.m^2]) */
    float izz;                 /**< Moment of inertia Izz  (measurment unit is [kg.m^2]) */
    float ixy;                 /**< Product of inertia Ixy  (measurment unit is [kg.m^2]) */
    float ixz;                 /**< Product of inertia Ixz  (measurment unit is [kg.m^2]) */
    float iyz;                 /**< Product of inertia Iyz  (measurment unit is [kg.m^2]) */
    int16 sunPointBodyVecX;    /**< Sun-pointing body vector X component  */
    int16 sunPointBodyVecY;    /**< Sun-pointing body vector Y component  */
    int16 sunPointBodyVecZ;    /**< Sun-pointing body vector Z component  */
    int16 tgtTrackBodyVecX;    /**< Target-tracking body vector X component  */
    int16 tgtTrackBodyVecY;    /**< Target-tracking body vector Y component  */
    int16 tgtTrackBodyVecZ;    /**< Target-tracking body vector Z component  */ //36 bytes, ID 189
} ADCS_AOD_t; // 164 Bytes


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                     ADCS Set Cmd Payload Structures                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct { // ID 2
    uint32 CurrentUnixseconds; // Current Unix time s. (Unit of measure is [s])
    uint32 CurrentUnixNanoseconds;  // Current Unix time ns. (Unit of measure is [ns])
}__attribute__((packed)) ADCS_CurrentUnixTimeCmd_Payload_t;

typedef struct { // ID 42
    uint8 ControlMode;
    uint8 MainEstimatorMode;
    uint8 BackupEstimatorMode;
    uint16 ControlTimeout;
}__attribute__((packed)) ADCS_ControlEstimationModeCmd_Payload_t;

typedef struct {// ID 48
    float TargetLatitude;
    float TargetLongiTude;
    float TargetAltitude;
}__attribute__((packed)) ADCS_ReferenceLLHTargetCmd_Payload_t;

typedef struct { // ID 51
    uint8 OrbitMode;
}__attribute__((packed)) ADCS_OrbitModeCmd_Payload_t;

typedef struct { // ID 54
    float Roll;
    float Pitch;
    float Yaw;
}__attribute__((packed)) ADCS_ReferenceRPYvaluesCmd_Payload_t;

typedef struct { // ID 68
    double Epoch;
    double Inclination;
    double RAAN;
    double Eccentricity;
    double AOP;
    double MeanAnomaly;
    double MeanMotion;
    double B_StarDrag;
}__attribute__((packed)) ADCS_SatOrbitParamConfigCmd_Payload_t;



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      ADCS Get Cmd Payload Structures                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
typedef struct { // ID 133
    uint32 CurrentUnixseconds; // Current Unix time s. (Unit of measure is [s])
    uint32 CurrentUnixNanoseconds;  // Current Unix time ns. (Unit of measure is [ns])
}__attribute__((packed)) ADCS_CurrentUnixTimeTlm_Payload_t;

typedef struct { // ID 150
    uint8 ControlMode;
    uint8 MainEstimatorMode;
    uint8 BackupEstimatorMode;
    uint16 ControlTimeout;
}__attribute__((packed)) ADCS_ControlEstimationModeTlm_Payload_t;

typedef struct { // ID 157
    float Latitude;
    float Longitude;
    float Altitude;
}__attribute__((packed)) ADCS_ReferenceLLHTargetTlm_Payload_t;

typedef struct { // ID 162
    uint8 OrbitMode;
}__attribute__((packed)) ADCS_OrbitModeTlm_Payload_t;

typedef struct { // ID 170
    uint32 TimeSecond;
    uint32 TimeNanoSecond;
    int16 FSS0AlphaAngle;
    int16 FSS0BetaAngle;
    uint8 FSS0CaptureResult;
    uint8 FSS0DetectionResult;
    int16 FSS1AlphaAngle;
    int16 FSS1BetaAngle;
    uint8 FSS1CaptureResult;
    uint8 FSS1DetectionResult;
    int16 FSS2AlphaAngle;
    int16 FSS2BetaAngle;
    uint8 FSS2CaptureResult;
    uint8 FSS2DetectionResult;
    int16 FSS3AlphaAngle;
    int16 FSS3BetaAngle;
    uint8 FSS3CaptureResult;
    uint8 FSS3DetectionResult;
    bool ValidResult; // FSS0, 1, 2, 3
}__attribute__((packed)) ADCS_RawCubeSenseSunTlm_Payload_t;

typedef struct { // ID 183
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
}__attribute__((packed)) ADCS_PowerStateTlm_Payload_t;

typedef struct { // ID 185
    /**
     * Not used in GroundCommand 
     * Only used in beacon
     */
    uint8 ControlMode;
    uint16 ControlTimeout;
}__attribute__((packed)) ADCS_ControlModeTlm_Payload_t;

typedef struct { // ID 196
    double Epoch;
    double Inclination;
    double RAAN;
    double Eccentricity;
    double AOP;
    double MeanAnomaly;
    double MeanMotion;
    double B_StarDrag;
}__attribute__((packed)) ADCS_SatOrbitParamConfigTlm_Payload_t;

typedef struct { // ID 203
    uint32 TimeSeconds;
    uint32 TimeNanoSeconds;
    uint8 CSS0;
    uint8 CSS1;
    uint8 CSS2;
    uint8 CSS3;
    uint8 CSS4;
    uint8 CSS5;
    uint8 CSS6;
    uint8 CSS7;
    uint8 CSS8;
    uint8 CSS9;
    bool CSSValidFlag;
}__attribute__((packed)) ADCS_RawCSSSensorTlm_Payload_t;

typedef struct { // ID 204
    uint32 TimeSeconds;
    uint32 TimeNanoSeconds;
    float GYR0RawRateX;
    float GYR0RawRateY;
    float GYR0RawRateZ;
    float GYR1RawRateX;
    float GYR1RawRateY;
    float GYR1RawRateZ;
    bool ValidFlag; // GYR0, 1
}__attribute__((packed)) ADCS_RawGYRSensorTlm_Paylaod_t;

typedef struct { // ID 207
    uint32 TimeSeconds;
    uint32 TimeNanoSeconds;
    float GYR0CalibratedRateX;
    float GYR0CalibratedRateY;
    float GYR0CalibratedRateZ;
    float GYR1CalibratedRateX;
    float GYR1CalibratedRateY;
    float GYR1CalibratedRateZ;
    float ExtGYR0CalibratedRateX;
    float ExtGYR0CalibratedRateY;
    float ExtGYR0CalibratedRateZ;
    float ExtGYR1CalibratedRateX;
    float ExtGYR1CalibratedRateY;
    float ExtGYR1CalibratedRateZ;
    bool ValidFlag; // GYR0, 1, ExtGYR0, 1
}__attribute__((packed)) ADCS_CalibratedGYRSensorTlm_Payload_t;


#endif