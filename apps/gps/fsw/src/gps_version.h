/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 * @file  GPS version information
 */
#ifndef GPS_VERSION_H
#define GPS_VERSION_H

#define GPS_BUILD_NUMBER    0
#define GPS_BUILD_BASELINE  "v7.0.0"
#define GPS_BUILD_DEV_CYCLE "v7.0.0"
#define GPS_BUILD_CODENAME  "Draco"

#define GPS_MAJOR_VERSION 7
#define GPS_MINOR_VERSION 0
#define GPS_REVISION      0

#define GPS_LAST_OFFICIAL "v7.0.0"

/* Mission revision: 1-254 reserved for mission patches (0 / 0xFF reserved) */
#define GPS_MISSION_REV 0x0

#define GPS_STR_HELPER(x) #x
#define GPS_STR(x)        GPS_STR_HELPER(x)

#define GPS_VERSION GPS_BUILD_BASELINE "+dev" GPS_STR(GPS_BUILD_NUMBER)

#define GPS_CFG_MAX_VERSION_STR_LEN 256

#endif /* GPS_VERSION_H */
