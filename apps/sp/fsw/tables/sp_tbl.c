#include "cfe_tbl_filedef.h"
#include "sp_tbl.h"

SP_APP_ExampleTable_t ExampleTable = {1, 2};

CFE_TBL_FILEDEF(ExampleTable, SP_APP.ExampleTable, Table Utility Test Table, sp_app_tbl.tbl)