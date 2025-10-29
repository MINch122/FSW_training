#ifndef CI_LAB_UTILS_H
#define CI_LBA_UTILS_H

#include "cfe.h"
#include "ci_lab_app.h"
#include "ci_lab_perfids.h"
#include "ci_lab_msgids.h"
#include "ci_lab_version.h"
#include "ci_lab_decode.h"

void CI_InitContactTime(void);
void CI_StoreContactTime(void);

void CI_SetEmissionMode(bool IsEmergency);

#endif