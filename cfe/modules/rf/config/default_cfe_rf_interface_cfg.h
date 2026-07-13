#ifndef CFE_RF_INTERFACE_CFG_H
#define CFE_RF_INTERFACE_CFG_H

#define CFE_RF_MAX_MISSING_TIME     (60 * 60 * 24 * 4)/* <\brief [sec] == 4 days */

/**
 * MAX RF MTU
 * This value only means the "user data" space size excluding various hdr, tail.
 * Differed by mission's RF configurations
 */
#define RF_MAX_MTU      215
#define RF_RPT_MAX_MTU  208

#endif