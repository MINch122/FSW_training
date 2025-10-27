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
 *
 * UEL_APP Application Mission Configuration Header File
 *
 * This is a compatibility header for the "mission_cfg.h" file that has
 * traditionally provided public config definitions for each CFS app.
 *
 * @note This file may be overridden/superceded by mission-provided defintions
 * either by overriding this header or by generating definitions from a command/data
 * dictionary tool.
 */
#ifndef UEL_APP_MISSION_CFG_H
#define UEL_APP_MISSION_CFG_H

#include "uel_app_interface_cfg.h"


#define UEL_APP_UEL_OBC_NODE                1
#define UEL_APP_UEL_CAM_NODE                2
#define UEL_APP_RED_UEL_OBC_NODE            7


#define UEL_APP_TERMINAL_CMD_MAX_LEN 128


/*
** CSP Port Definitions
*/
#define UEL_APP_PORT                25  /**< Default UEL communication port */

#define UEL_APP_ID_GetSensData      0x20 /**< 센서 데이터 요청 */
#define UEL_APP_ID_SetCamPower      0x21 /**< 카메라 전원 제어 */
#define UEL_APP_ID_SetMotorMode     0x23 /**< 모터 모드 설정 */
#define UEL_APP_ID_SetCamShotCmd    0x41 /**< 카메라 촬영 요청 */
#define UEL_APP_ID_GetCamShotStat   0x43 /**< 촬영 완료 상태 조회 */
#define UEL_APP_ID_GetCamImage      0x44 /**< 이미지 데이터 요청 */
#define UEL_APP_ID_SendBcn          0x45 // 비콘 송신
#define UEL_APP_ID_SetTerminalCmd   0x46//터미널 커맨드 실행







#endif
