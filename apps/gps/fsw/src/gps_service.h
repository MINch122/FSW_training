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
 * @file  GPS receiver service: device ownership, receive task, position cache
 */
#ifndef GPS_SERVICE_H
#define GPS_SERVICE_H

#include "cfe.h"
#include "gps_msg.h"

/** Open the device and start the receive task. */
CFE_Status_t GPS_ServiceInit(void);

/**
 * Stop the receive task and release the device.
 *
 * Waits for the task to leave the driver before freeing what it reads from;
 * the interface is left allocated if the task does not come back in time.
 */
void GPS_ServiceShutdown(void);

/**
 * Copy the receive-path counters into @a Hk.
 *
 * The receive task owns those fields, so they are only safe to read through
 * here. The command counters in @a Hk are left untouched.
 */
void GPS_ServiceGetCounters(GPS_HkTlm_Payload_t* Hk);

/** Zero the receive-path counters. */
void GPS_ServiceResetCounters(void);

#endif /* GPS_SERVICE_H */
