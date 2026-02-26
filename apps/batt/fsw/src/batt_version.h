/**
 * @file
 *
 *  The BATT App header file containing version information
 */

#ifndef BATT_VERSION_H
#define BATT_VERSION_H

/* Development Build Macro Definitions */

#define BATT_BUILD_NUMBER    1 /*!< Development Build: Number of commits since baseline */
#define BATT_BUILD_BASELINE  "v1.0.0-dev"
#define BATT_BUILD_DEV_CYCLE "v1.0.0"
#define BATT_BUILD_CODENAME  "NanoPowerBP8"

/*
 * Version Macros
 */
#define BATT_MAJOR_VERSION 1
#define BATT_MINOR_VERSION 0
#define BATT_REVISION      0

#define BATT_LAST_OFFICIAL "v1.0.0"

#define BATT_MISSION_REV 0xFF

#define BATT_STR_HELPER(x) #x
#define BATT_STR(x) BATT_STR_HELPER(x)

#define BATT_VERSION BATT_BUILD_BASELINE "+dev" BATT_STR(BATT_BUILD_NUMBER)

#define BATT_CFG_MAX_VERSION_STR_LEN 256

#endif /* BATT_VERSION_H */
