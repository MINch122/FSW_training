/**
 * Coverage tests for meow_sys.c — pure POSIX.
 * meow_sys_shutdown and meow_sys_force_kill are NOT tested here —
 * require container/VM isolation; run separately in a safe environment.
 */

#include "utassert.h"
#include "uttest.h"

#include "meow_sys.h"

#include <string.h>

/* -------------------------------------------------------------------------
 * meow_sys_info
 * ---------------------------------------------------------------------- */

void Test_SysInfo_NullArg(void)
{
    UtAssert_INT32_EQ(meow_sys_info(NULL), MEOW_SYS_ERR_NULL);
}

void Test_SysInfo_Success(void)
{
    meow_sys_info_t info;
    memset(&info, 0, sizeof(info));
    UtAssert_INT32_EQ(meow_sys_info(&info), MEOW_SYS_OK);
    UtAssert_True(info.uptime_sec > 0, "uptime_sec > 0");
    UtAssert_True(info.total_ram > 0,  "total_ram > 0");
    UtAssert_True(info.num_procs > 0,  "num_procs > 0");
}

/* -------------------------------------------------------------------------
 * meow_sys_sync
 * ---------------------------------------------------------------------- */

void Test_SysSync_AlwaysOK(void)
{
    UtAssert_INT32_EQ(meow_sys_sync(), MEOW_SYS_OK);
}

/* -------------------------------------------------------------------------
 * meow_sys_time_get
 * ---------------------------------------------------------------------- */

void Test_SysTimeGet_NullArg(void)
{
    UtAssert_INT32_EQ(meow_sys_time_get(NULL), MEOW_SYS_ERR_NULL);
}

void Test_SysTimeGet_Success(void)
{
    meow_sys_time_t t;
    memset(&t, 0, sizeof(t));
    UtAssert_INT32_EQ(meow_sys_time_get(&t), MEOW_SYS_OK);
    UtAssert_True(t.sec > 0, "sec > 0 (post-epoch)");
}

/* -------------------------------------------------------------------------
 * meow_sys_time_set (null rejection only; actual set requires CAP_SYS_TIME)
 * ---------------------------------------------------------------------- */

void Test_SysTimeSet_NullArg(void)
{
    UtAssert_INT32_EQ(meow_sys_time_set(NULL), MEOW_SYS_ERR_NULL);
}

/* -------------------------------------------------------------------------
 * Test registration
 * ---------------------------------------------------------------------- */

void UtTest_Setup(void)
{
    UtTest_Add(Test_SysInfo_NullArg,          NULL, NULL, "SysInfo_NullArg");
    UtTest_Add(Test_SysInfo_Success,          NULL, NULL, "SysInfo_Success");
    UtTest_Add(Test_SysSync_AlwaysOK,         NULL, NULL, "SysSync_AlwaysOK");
    UtTest_Add(Test_SysTimeGet_NullArg,       NULL, NULL, "SysTimeGet_NullArg");
    UtTest_Add(Test_SysTimeGet_Success,       NULL, NULL, "SysTimeGet_Success");
    UtTest_Add(Test_SysTimeSet_NullArg,       NULL, NULL, "SysTimeSet_NullArg");
}
