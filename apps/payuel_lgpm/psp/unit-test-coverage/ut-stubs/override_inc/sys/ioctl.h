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
#ifndef OVERRIDE_IOCTL_H
#define OVERRIDE_IOCTL_H

#include "PCS_ioctl.h"

/* ----------------------------------------- */
/* mappings for declarations in ioLib.h */
/* ----------------------------------------- */
#define SIOCGIFINDEX                PCS_SIOCGIFINDEX
#define SPI_IOC_MESSAGE(N)          PCS_SPI_IOC_MESSAGE(N)
#define SPI_IOC_WR_MODE             PCS_SPI_IOC_WR_MODE
#define SPI_IOC_WR_MAX_SPEED_HZ     PCS_SPI_IOC_WR_MAX_SPEED_HZ
#define SPI_IOC_WR_BITS_PER_WORD    PCS_SPI_IOC_WR_BITS_PER_WORD
#define TCFLSH                      PCS_TCFLSH
#define TCGETS2                     PCS_TCGETS2
#define TCSETS2                     PCS_TCSETS2

#define I2C_SLAVE                   PCS_I2C_SLAVE
#define I2C_TIMEOUT                 PCS_I2C_TIMEOUT
#define I2C_RDWR                    PCS_I2C_RDWR
#define I2C_TENBIT                  PCS_I2C_TENBIT
#define I2C_RETRIES                 PCS_I2C_RETRIES
#define I2C_PEC                     PCS_I2C_PEC

#define ioctl   PCS_ioctl
#endif
