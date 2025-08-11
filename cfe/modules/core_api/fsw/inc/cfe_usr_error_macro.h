#ifndef CFE_USR_ERROR_MACRO_H
#define CFE_USR_ERROR_MACRO_H


/**
 * User Moule Identifiers
 * Srv bit should be `000`
 * Revise Mission defined code
 * `srl` = `0b000000001`
 * `rf` = `0b000000010`
 */
#define CFE_CODE_BITS        16
#define CFE_MISSION_BITS     9
#define CFE_SERVICE_BITS     3
#define CFE_RESERVE_BITS     2
#define CFE_SEVERITY_BITS    2

#define CFE_MISSION_SHIFT   CFE_CODE_BITS
#define CFE_SERVICE_SHIFT   (CFE_MISSION_SHIFT + CFE_MISSION_BITS)
#define CFE_RESERVE_SHIFT   (CFE_SERVICE_SHIFT + CFE_SERVICE_BITS)
#define CFE_SEVERITY_SHIFT  (CFE_RESERVE_SHIFT + CFE_RESERVE_BITS)
   

#define CFE_MISSION_DEFINED_BITMASK     ((CFE_Status_t)0x01ff0000)
#define CFE_MISSION_ID_SERIAL              0x001
#define CFE_MISSION_ID_RADIO               0x002

/**
 * Macro for user core status code
 * `s`: Severity
 * `m`: Mission defined id
 * `c`: Specific code
 */
#define CFE_USER_MODULE_CODE(s, m, c)\
        (((s)<<CFE_SEVERITY_SHIFT)|(0<<CFE_RESERVE_SHIFT)|\
        (0<<CFE_SERVICE_SHIFT)|(((m) & 0x1FF)<<CFE_MISSION_SHIFT)|\
        ((c) & 0xFFFF))

/**
 * User API macro for Status code generation  
 */
#define CFE_SERIAL_INFORMATION(n)   CFE_STATUS_C(CFE_USER_MODULE_CODE(1, CFE_MISSION_ID_SERIAL, (n)))
#define CFE_SERIAL_ERROR(n)         CFE_STATUS_C(CFE_USER_MODULE_CODE(3, CFE_MISSION_ID_SERIAL, (n)))
#define CFE_RADIO_INFORMATION(n)    CFE_STATUS_C(CFE_USER_MODULE_CODE(1, CFE_MISSION_ID_RADIO, (n)))
#define CFE_RADIO_ERROR(n)          CFE_STATUS_C(CFE_USER_MODULE_CODE(3, CFE_MISSION_ID_RADIO, (n)))

#endif /* CFE_USR_ERROR_MACRO_H */