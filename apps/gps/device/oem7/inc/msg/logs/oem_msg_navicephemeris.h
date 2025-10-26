#ifndef _OEM_MSG_NAVICEPHEMERIS_H_
#define _OEM_MSG_NAVICEPHEMERIS_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_NAVICEPHEMERIS 2123

/**
 * @struct oem_log_navicephemeris
 *
 * @brief Decoded NavIC Ephemeris. Asynch log. MID = 2123.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif

    /**
     * @brief Satellite Identifier (1 to 7).
     */
    oem_ulong           prn;

    /**
     * @brief Week number since August 22nd 1999.
     */
    oem_ulong           wn;

    /**
     * @brief Clock bias (sec).
     */
    oem_double          af0;

    /**
     * @brief Clock drift (sec/sec).
     */
    oem_double          af1;

    /**
     * @brief Clock drift rate (sec/sec^2).
     */
    oem_double          af2;

    /**
     * @brief SV accuracy.
     */
    oem_ulong           ura;

    /**
     * @brief Reference time for the satellite clock corrections (sec).
     */
    oem_ulong           toc;

    /**
     * @brief Total group delay (sec).
     */
    oem_double          tgd;

    /**
     * @brief Mean motion difference (radian/sec).
     */
    oem_double          deltaN;

    /**
     * @brief Issue of data ephemeris and clock.
     */
    oem_ulong           iodec;

    /** @brief unused. */
    oem_ulong           reserved;

    /**
     * @brief Health status of navigation data on L5 SPS signal (0=OK, 1=BAD).
     */
    oem_ulong           L5Health;

    /**
     * @brief Health status of navigation data on S SPS signal (0=OK, 1=BAD).
     */
    oem_ulong           SHealth;

    /**
     * @brief Amplitude of the cosine harmonic correction term to the
     *        argument of latitude (radians).
     */
    oem_double          cuc;

    /**
     * @brief Amplitude of the sine harmonic correction term to the
     *        argument of latitude (radians).
     */
    oem_double          cus;

    /**
     * @brief Amplitude of the cosine harmonic correction term to the
     *        angle of inclination (radians).
     */
    oem_double          cic;

    /**
     * @brief Amplitude of the sine harmonic correction term to the
     *        angle of inclination (radians).
     */
    oem_double          cis;

    /**
     * @brief Amplitude of the cosine harmonic correction term to the
     *        orbit radius (m).
     */
    oem_double          crc;

    /**
     * @brief Amplitude of the sine harmonic correction term to the
     *        orbit radius (m).
     */
    oem_double          crs;

    /**
     * @brief Rate of inclination angle (radians/sec).
     */
    oem_double          idot;

    /** @brief unused. */
    oem_ulong           spare;

    /**
     * @brief Mean anomaly (radians).
     */
    oem_double          M0;

    /**
     * @brief Time of ephemeris (sec).
     */
    oem_ulong           toe;

    /**
     * @brief Eccentricity.
     */
    oem_double          ecc;

    /**
     * @brief Square root of semi-major axis (sqrt(m)).
     */
    oem_double          rootA;

    /**
     * @brief Longitude of ascending node (radians).
     */
    oem_double          omega0;

    /**
     * @brief Argument of perigee (radians).
     */
    oem_double          omega;

    /**
     * @brief Rate of RAAN (radians/sec).
     */
    oem_double          omegaDot;

    /**
     * @brief Inclination angle (radians).
     */
    oem_double          i0;

    /** @brief unused. */
    oem_ulong           spare2;

    /**
     * @brief 1=Alert; 0=OK.
     */
    oem_ulong           alertFlag;

    /**
     * @brief When set to 1, satellite is in AutoNav mode.
     */
    oem_ulong           autoNavFlag;

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_navicephemeris;

#endif