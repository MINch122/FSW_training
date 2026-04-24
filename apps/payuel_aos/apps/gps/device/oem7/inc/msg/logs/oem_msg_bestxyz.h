#ifndef _OEM_MSG_BESTXYZ_H_
#define _OEM_MSG_BESTXYZ_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_BESTXYZ 241

/**
 * @struct oem_log_bestxyz
 *
 * @brief Best available cartesian position and velocity. Synch log. MID = 241.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /**
     * @brief Solution status. See Table 73 (Solution Status).
     */
    oem_enum          pSolStatus;

    /**
     * @brief Position type. See Table 74 (Position of Velocity Type).
     */
    oem_enum          posType;

    /**
     * @brief ECEF X-coordinate (m).
     */
    oem_double        pX;

    /**
     * @brief ECEF Y-coordinate (m).
     */
    oem_double        pY;

    /**
     * @brief ECEF Z-coordinate (m).
     */
    oem_double        pZ;

    /**
     * @brief Standard deviation of pX (m).
     */
    oem_float         pXstd;

    /**
     * @brief Standard deviation of pY (m).
     */
    oem_float         pYstd;

    /**
     * @brief Standard deviation of pZ (m).
     */
    oem_float         pZstd;

    /**
     * @brief Solution status. See Table 73 (Solution Status).
     */
    oem_enum          vSolStatus;

    /**
     * @brief Velocity type. See Table 74 (Position of Velocity Type).
     */
    oem_enum          velType;

    /**
     * @brief ECEF velocity vector along X-axis (m/s).
     */
    oem_double        vX;

    /**
     * @brief ECEF velocity vector along Y-axis (m/s).
     */
    oem_double        vY;

    /**
     * @brief ECEF velocity vector along Z-axis (m/s).
     */
    oem_double        vZ;

    /**
     * @brief Standard deviation of vX (m/s).
     */
    oem_float         vXstd;

    /**
     * @brief Standard deviation of vY (m/s).
     */
    oem_float         vYstd;

    /**
     * @brief Standard deviation of vZ (m/s).
     */
    oem_float         vZstd;

    /**
     * @brief Base station identification.
     */
    oem_char          stnId[4];

    /**
     * @brief Latency in the velocity time tag in seconds. It should 
     *        be subtracted from the time to give improved results.
     */
    oem_float         vLatancy;

    /**
     * @brief Differential age in seconds.
     */
    oem_float         diffAge;

    /**
     * @brief Solution age in seconds.
     */
    oem_float         solAge;

    /**
     * @brief # of satellites tracked.
     */
    oem_uchar         numSats;

    /**
     * @brief # of satellite vehicles used in solution.
     */
    oem_uchar         numSolnSats;

    /**
     * @brief # of GPS plus GLONASS plus BDS L1/B1 used in solution.
     */
    oem_uchar         ggL1;

    /**
     * @brief # of satellites with L1/E1/B1 signals used in solution.
     */
    oem_uchar         numSolnMultiSats;

    /**
     * @brief Unused.
     */
    oem_char          reserved;

    /**
     * @brief Extended solution status. See Table 77.
     */
    oem_uchar         extSolStat;

    /**
     * @brief Galileo and BeiDou signals used mask. See Table 76.
     */
    oem_uchar         galileoAndBeiduSigMask;

    /**
     * @brief GPS and GLONASS signals used mask. See Table 75.
     */
    oem_uchar         gpsAndGlonassSigMask;

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_bestxyz;


typedef enum {
    OEM_BESTXYZ_SOLSTAT_SOL_COMPUTED        = 0,
    OEM_BESTXYZ_SOLSTAT_INSUFFICIENT_OBS    = 1,
    OEM_BESTXYZ_SOLSTAT_NO_CONVERGENCE      = 2,
    OEM_BESTXYZ_SOLSTAT_SINGULARITY         = 3,
    OEM_BESTXYZ_SOLSTAT_COV_TRACE           = 4,
    OEM_BESTXYZ_SOLSTAT_TEST_DIST           = 5,
    OEM_BESTXYZ_SOLSTAT_COLD_START          = 6,
    OEM_BESTXYZ_SOLSTAT_V_H_LIMIT           = 7,
    OEM_BESTXYZ_SOLSTAT_VARIANCE            = 8,
    OEM_BESTXYZ_SOLSTAT_RESIDUALS           = 9,
    OEM_BESTXYZ_SOLSTAT_INTEGRITY_WARNING   = 10,
    OEM_BESTXYZ_SOLSTAT_PENDING             = 18,
    OEM_BESTXYZ_SOLSTAT_INVALID_FIX         = 19,
    OEM_BESTXYZ_SOLSTAT_UNAUTHORIZED        = 20,
    OEM_BESTXYZ_SOLSTAT_INVALID_RATE        = 22,
} oem_bestxyz_sol_status_t;


#endif
