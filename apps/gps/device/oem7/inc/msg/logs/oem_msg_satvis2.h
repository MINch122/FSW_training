#ifndef _OEM_MSG_SATVIS2_H_
#define _OEM_MSG_SATVIS2_H_

#include "msg/oem_msg_common.h"

#define OEM_ID_LOG_SATVIS2 1043

/**
 * @struct oem_log_satvis2_comp
 *
 * @brief Visibility of one satellite. Positions come from the almanac, not
 *        from the ephemeris, so this is an overview rather than a fix input.
 */
typedef struct {
    /**
     * @brief Satellite ID. Low 2 bytes are the system identifier (PRN for
     *        GPS, slot for GLONASS), high 2 bytes are the GLONASS frequency
     *        channel as a signed short, zero for every other system.
     */
    oem_ulong           satId;
    /**
     * @brief Satellite health. See the interface control document of the
     *        satellite system.
     */
    oem_ulong           health;
    /**
     * @brief Elevation (degrees).
     */
    oem_double          elev;
    /**
     * @brief Azimuth (degrees).
     */
    oem_double          az;
    /**
     * @brief Theoretical Doppler of the satellite (Hz).
     */
    oem_double          trueDop;
    /**
     * @brief Apparent Doppler for this receiver, clock drift included (Hz).
     */
    oem_double          appDop;

} OEM_PACK oem_log_satvis2_comp;

/**
 * @struct oem_log_satvis2
 *
 * @brief Satellite visibility, one log per satellite system. Asynch log,
 *        emitted at most every 10 s by the receiver. MID = 1043.
 */
typedef struct {
#ifdef OEM_MSG_INCLUDE_BIN_HEADER
    oem_binary_header_t header;
#endif
    /**
     * @brief GNSS satellite system identifier. See Table 102 (Satellite
     *        System) of the manual.
     */
    oem_enum            satSystem;
    /**
     * @brief Is the satellite visibility valid? 0 = FALSE, 1 = TRUE.
     */
    oem_enum            satVis;
    /**
     * @brief Was a complete almanac used? 0 = FALSE, 1 = TRUE.
     */
    oem_enum            compAlm;
    /**
     * @brief # of satellites with data to follow.
     */
    oem_ulong           numSat;
    /**
     * @brief Per-satellite visibility. See oem_log_satvis2_comp.
     */
    oem_log_satvis2_comp comp[];

    /* 32-bit CRC at the end */
} OEM_PACK oem_log_satvis2;

#endif
