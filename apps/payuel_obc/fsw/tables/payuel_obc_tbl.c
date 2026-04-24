#include "cfe_tbl_filedef.h"
#include "payuel_obc_tbl.h"

PAYUEL_OBC_ExampleTable_t ExampleTable = {1, 2};

CFE_TBL_FILEDEF(ExampleTable, PAYUEL_OBC.ExampleTable, PAYUEL_OBC Table, payuel_obc_tbl.tbl)
