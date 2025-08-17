#ifndef CFE_USR_MACRO_H
#define CFE_USR_MACRO_H

/***********************************************************
 * User macro
 * Used for put some value to HK structure (which is packed)
 ***********************************************************/
#define CFE_PUT_VALUE_TO_STRUCT(type, addr, func, success_expr)    \
    ({                                      \
        type __tmp;                         \
        int __result = func(&__tmp);        \
        if (__result == (success_expr))     \
            memcpy(addr, &__tmp, sizeof(type)); \
        __result;                           \
    })

#endif