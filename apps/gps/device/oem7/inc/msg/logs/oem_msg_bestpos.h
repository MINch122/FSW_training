#ifndef _OEM_MSG_BESTPOS_H_
#define _OEM_MSG_BESTPOS_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_BESTPOS 42

/**
 * @struct oem_log_bestpos
 *
 * @brief Best available position in geodetic coordinates. Synch log. MID = 42.
 *        Body is a fixed 72 bytes.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /** @brief Solution status. See Table 73 (Solution Status). */
    oem_enum    solStatus;

    /** @brief Position type. See Table 74 (Position or Velocity Type). */
    oem_enum    posType;

    /** @brief Latitude (degrees), positive north. */
    oem_double  lat;

    /** @brief Longitude (degrees), positive east. */
    oem_double  lon;

    /**
     * @brief Height above mean sea level (m). Add @a undulation for the
     *        height above the WGS84 ellipsoid.
     */
    oem_double  hgt;

    /** @brief Geoid separation (m): ellipsoidal height minus @a hgt. */
    oem_float   undulation;

    /** @brief Datum ID. 61 = WGS84. */
    oem_enum    datumId;

    /** @brief Standard deviation of lat (m). */
    oem_float   latStd;

    /** @brief Standard deviation of lon (m). */
    oem_float   lonStd;

    /** @brief Standard deviation of hgt (m). */
    oem_float   hgtStd;

    /** @brief Base station identification. */
    oem_char    stnId[4];

    /** @brief Differential age (s). */
    oem_float   diffAge;

    /** @brief Solution age (s). */
    oem_float   solAge;

    /** @brief # of satellites tracked. */
    oem_uchar   numSats;

    /** @brief # of satellites used in the solution. */
    oem_uchar   numSolnSats;

    /** @brief # of satellites with L1/E1/B1 signals used in the solution. */
    oem_uchar   numSolnL1Sats;

    /** @brief # of satellites with multi-frequency signals used. */
    oem_uchar   numSolnMultiSats;

    /** @brief Unused. */
    oem_uchar   reserved;

    /** @brief Extended solution status. See Table 77. */
    oem_uchar   extSolStat;

    /** @brief Galileo and BeiDou signals used mask. See Table 76. */
    oem_uchar   galileoAndBeidouSigMask;

    /** @brief GPS and GLONASS signals used mask. See Table 75. */
    oem_uchar   gpsAndGlonassSigMask;

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_bestpos;

typedef enum {
    OEM_BESTPOS_SOLSTAT_SOL_COMPUTED      = 0,
    OEM_BESTPOS_SOLSTAT_INSUFFICIENT_OBS  = 1,
    OEM_BESTPOS_SOLSTAT_NO_CONVERGENCE    = 2,
    OEM_BESTPOS_SOLSTAT_SINGULARITY       = 3,
    OEM_BESTPOS_SOLSTAT_COV_TRACE         = 4,
    OEM_BESTPOS_SOLSTAT_TEST_DIST         = 5,
    OEM_BESTPOS_SOLSTAT_COLD_START        = 6,
    OEM_BESTPOS_SOLSTAT_V_H_LIMIT         = 7,
    OEM_BESTPOS_SOLSTAT_VARIANCE          = 8,
    OEM_BESTPOS_SOLSTAT_RESIDUALS         = 9,
    OEM_BESTPOS_SOLSTAT_INTEGRITY_WARNING = 10,
    OEM_BESTPOS_SOLSTAT_PENDING           = 18,
    OEM_BESTPOS_SOLSTAT_INVALID_FIX       = 19,
    OEM_BESTPOS_SOLSTAT_UNAUTHORIZED      = 20,
    OEM_BESTPOS_SOLSTAT_INVALID_RATE      = 22,
} oem_bestpos_sol_status_t;

#endif
