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

/* PSP coverage stub replacement for ioLib.h */
#ifndef PCS_IOCTL_H
#define PCS_IOCTL_H

#include "PCS_basetypes.h"

/* ----------------------------------------- */
/* constants normally defined in ioLib.h */
/* ----------------------------------------- */
#define PCS_SIOCGIFINDEX                0x136
#define PCS_SPI_IOC_MESSAGE(N)          0x283
#define PCS_SPI_IOC_WR_MODE             0x284
#define PCS_SPI_IOC_WR_MAX_SPEED_HZ     0x287
#define PCS_SPI_IOC_WR_BITS_PER_WORD    0x334
#define PCS_TCFLSH                      0x361
#define PCS_TCGETS2                     0x261
#define PCS_TCSETS2                     0x331

#define PCS_I2C_SLAVE                   0x466
#define PCS_I2C_TIMEOUT                 0x488
#define PCS_I2C_RDWR                    0x491
#define PCS_I2C_TENBIT                  0x626
#define PCS_I2C_RETRIES                 0x364
#define PCS_I2C_PEC                     0x250
/* ----------------------------------------- */
/* types normally defined in ioLib.h */
/* ----------------------------------------- */

/* ----------------------------------------- */
/* prototypes normally declared in ioLib.h */
/* ----------------------------------------- */
extern int PCS_ioctl(int fd, unsigned long request, ...);

#endif
