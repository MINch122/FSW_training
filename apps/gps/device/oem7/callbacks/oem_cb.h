#ifndef _OEM_CB_H_
#define _OEM_CB_H_

#define OEM_CALLBACK_LOG_SAVE_PATH_ROOT  "/cf/sdcard/"

int oem_callback_VERSION_print(void* msg);
int oem_callback_HWMONITOR_print(void* msg);
int oem_callback_BESTXYZ_binfile(void* msg);

#endif
