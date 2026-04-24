#include "cfe_tbl_filedef.h"
#include "payuel_cam_tbl.h"

PAYUEL_CAM_ExampleTable_t ExampleTable = {1, 2};

CFE_TBL_FILEDEF(ExampleTable, PAYUEL_CAM.ExampleTable, PAYUEL_CAM Table, payuel_cam_tbl.tbl)
