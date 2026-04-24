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
 * \file
 * \ingroup ut-stubs
 *
 * OSAL coverage stub replacement for poll.h
 */

#ifndef OCS_POLL_H
#define OCS_POLL_H

#include "OCS_basetypes.h"
// #include <poll.h>

/* ----------------------------------------- */
/* constants normally defined in poll.h */
/* ----------------------------------------- */
#define OCS_POLLIN  0x488

#define OCS_POLLERR 0x491
#define OCS_POLLHUP 0x334
#define OCS_POLLNVAL 0x287


/* ----------------------------------------- */
/* types normally defined in poll.h */
/* ----------------------------------------- */
struct OCS_pollfd {
    int fd;
    short int events;
    short int revents;
};

struct OCS_nfds {
    int nfds;
};

typedef struct OCS_pollfd   OCS_pollfd_t;
typedef struct OCS_nfds     OCS_nfds_t;


/* ----------------------------------------- */
/* prototypes normally declared in poll.h */
/* ----------------------------------------- */
extern int OCS_poll(OCS_pollfd_t fds, OCS_nfds_t nfds, int timeout);

#endif /* OCS_POLL_H */
