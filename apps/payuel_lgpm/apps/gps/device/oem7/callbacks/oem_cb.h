#ifndef _OEM_CB_H_
#define _OEM_CB_H_

#define OEM_CALLBACK_LOG_SAVE_PATH_ROOT  "/cf/sdcard/"

int OEM_Callback_VERSION(void* msg);
int OEM_Callback_HWMONITOR(void* msg);
int OEM_Callback_BESTXYZ_Bin(void* msg);

#endif
