#ifndef FM_MSGSTRUCT_H
#define FM_MSGSTRUCT_H

/************************************************************************
 * NASA Docket No. GSC-18,918-1, and identified as “Core Flight
 * Software System (cFS) File Manager Application Version 2.6.1”
 *
 * Copyright (c) 2021 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   Specification for the CFS FM command and telemetry messages.
 */
#include "cfe.h"
#include "fm_platform_cfg.h"
#include "fm_extern_typedefs.h"
#include "fm_msgdefs.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM -- command packet structures                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * \defgroup cfsfmcmdstructs CFS File Manager Command Structures
 * \{
 */

/**
 *  \brief Housekeeping Request command packet structure
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} FM_SendHkCmd_t;

/**
 *  \brief No-Operation command packet structure
 *
 *  For command details see #FM_NOOP_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} FM_NoopCmd_t;

/**
 *  \brief Reset Counters command packet structure
 *
 *  For command details see #FM_RESET_COUNTERS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} FM_ResetCountersCmd_t;


/**
 *  \brief Copy File command packet structure
 *
 *  For command details see #FM_COPY_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_OvwSourceTargetFilename_Payload_t Payload; /**< \brief Command payload */
} FM_CopyFileCmd_t;

/**
 *  \brief Move File command packet structure
 *
 *  For command details see #FM_MOVE_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_OvwSourceTargetFilename_Payload_t Payload; /**< \brief Command payload */
} FM_MoveFileCmd_t;


/**
 *  \brief Rename File command packet structure
 *
 *  For command details see #FM_RENAME_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_SourceTargetFileName_Payload_t Payload; /**< \brief Command payload */
} FM_RenameFileCmd_t;


/**
 *  \brief Delete File command packet structure
 *
 *  For command details see #FM_DELETE_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_SingleFilename_Payload_t Payload; /**< \brief Command Payload */
} FM_DeleteFileCmd_t;


/**
 *  \brief Delete All command packet structure
 *
 *  For command details see #FM_DELETE_ALL_FILES_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_DirectoryName_Payload_t Payload; /**< \brief Command Payload */
} FM_DeleteAllFilesCmd_t;

/**
 *  \brief Decompress File command packet structure
 *
 *  For command details see #FM_DECOMPRESS_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_SourceTargetFileName_Payload_t Payload; /**< \brief Command Payload */
} FM_DecompressFileCmd_t;


/**
 *  \brief Concatenate Files command packet structure
 *
 *  For command details see #FM_CONCAT_FILES_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_TwoSourceOneTarget_Payload_t Payload; /**< \brief Command Payload */
} FM_ConcatFilesCmd_t;


/**
 *  \brief Get File Info command packet structure
 *
 *  For command details see #FM_GET_FILE_INFO_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_FilenameAndCRC_Payload_t Payload; /**< \brief Command Payload */
} FM_GetFileInfoCmd_t;

/**
 *  \brief Get Open Files command packet structure
 *
 *  For command details see #FM_GET_OPEN_FILES_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} FM_GetOpenFilesCmd_t;

/**
 *  \brief Create Directory command packet structure
 *
 *  For command details see #FM_CREATE_DIRECTORY_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_DirectoryName_Payload_t Payload; /**< \brief Command Payload */
} FM_CreateDirectoryCmd_t;

/**
 *  \brief Delete Directory command packet structure
 *
 *  For command details see #FM_DELETE_DIRECTORY_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_DirectoryName_Payload_t Payload; /**< \brief Command Payload */
} FM_DeleteDirectoryCmd_t;


/**
 *  \brief Get DIR List to File command packet structure
 *
 *  For command details see #FM_GET_DIR_LIST_FILE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_GetDirectoryToFile_Payload_t Payload; /**< \brief Command Payload */
} FM_GetDirListFileCmd_t;


/**
 *  \brief Get DIR List to Packet command packet structure
 *
 *  For command details see #FM_GET_DIR_LIST_PKT_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_GetDirectoryToPkt_Payload_t Payload; /**< \brief Command Payload */
} FM_GetDirListPktCmd_t;

/**
 *  \brief Get Free Space command packet structure
 *
 *  For command details see #FM_MONITOR_FILESYSTEM_SPACE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} FM_MonitorFilesystemSpaceCmd_t;


/**
 *  \brief Set Table State command packet structure
 *
 *  For command details see #FM_SET_TABLE_STATE_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_TableIndexAndState_Payload_t Payload; /**< \brief Command Payload */
} FM_SetTableStateCmd_t;


/**
 *  \brief Set Permissions for a file
 *
 *  For command details see #FM_SET_PERMISSIONS_CC
 */
typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */

    FM_FilenameAndMode_Payload_t Payload;
} FM_SetPermissionsCmd_t;

/**\}*/

/**
 * \defgroup cfsfmtlm CFS File Manager Telemetry
 * \{
 */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM -- get directory listing telemetry structures                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 *  \brief Get Directory Listing telemetry packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry Header */

    FM_DirListPkt_Payload_t Payload; /**< \brief Telemetry Payload */
} FM_DirListPkt_t;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM -- get directory listing to file structures                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 *  \brief Get Directory Listing file statistics structure
 */
typedef struct
{
    char   DirName[OS_MAX_PATH_LEN]; /**< \brief Directory name */
    uint32 DirEntries;               /**< \brief Number of entries in the directory */
    uint32 FileEntries;              /**< \brief Number of entries written to output file */
} FM_DirListFileStats_t;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM -- get file information telemetry structure                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
 *  \brief Get File Info telemetry packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry Header */

    FM_FileInfoPkt_Payload_t Payload; /**< \brief Telemetry Payload */
} FM_FileInfoPkt_t;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM -- get open files list telemetry structures                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 *  \brief Get Open Files telemetry packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry Header */

    FM_OpenFilesPkt_Payload_t Payload; /**< \brief Telemetry Payload */
} FM_OpenFilesPkt_t;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM -- monitor filesystem telemetry structures                   */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 *  \brief Monitor filesystem telemetry packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry Header */

    FM_MonitorReportPkt_Payload_t Payload; /**< \brief Telemetry Payload */
} FM_MonitorReportPkt_t;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM -- housekeeping telemetry structure                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * \brief Housekeeping telemetry packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry Header */

    FM_HousekeepingPkt_Payload_t Payload; /**< \brief Telemetry Payload */
} FM_HousekeepingPkt_t;

/**\}*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM -- monitor filesyste table structures                        */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef enum
{
    /**
     * Table entry is not used, these entries are ignored
     */
    FM_MonitorTableEntry_Type_UNUSED = 0,

    /**
     * Monitor the free space on given volume
     *
     * The given path will be passed to OS_FileSysStatVolume() and the results
     * will be reported in the generated TLM entry.
     */
    FM_MonitorTableEntry_Type_VOLUME_FREE_SPACE = 1,

    /**
     * Estimate the sum of space used by files within specified directory
     *
     * The given path will be opened as a directory.  The size of each regular
     * file present in that directory will be summed to produce an estimate of the
     * total space associated with that directory.
     *
     * Note that this yields only an estimate, as there can be discrepancies
     * between the file size as observed by this method and the actual disk blocks
     * used by a given file.
     */
    FM_MonitorTableEntry_Type_DIRECTORY_ESTIMATE = 2
} FM_MonitorTableEntry_Type_t;

/**
 *  \brief Monitor table entry
 */
typedef struct
{
    /**
     * Table entry type.
     *
     * This should be one of the enumeration values in FM_MonitorTableEntry_Type_t.
     * It is defined as a uint8 in this table to ensure a consistent size.
     */
    uint8_t Type;

    /**
     * Boolean flag indicating whether this entry is active or not
     */
    uint8_t Enabled;

    /**
     * Location to monitor
     *
     * The interpretation of this string depends on Type
     * See description of the FM_MonitorTableEntry_Type_t for how this is to be set
     */
    char Name[OS_MAX_PATH_LEN];
} FM_MonitorTableEntry_t;

/**
 *  \brief Get Free Space table definition
 */
typedef struct
{
    FM_MonitorTableEntry_t Entries[FM_TABLE_ENTRY_COUNT]; /**< \brief One entry for each monitor */
} FM_MonitorTable_t;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* FM -- child task interface command queue entry                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 *  \brief Child Task Interface command queue entry structure
 */
typedef struct
{
    CFE_MSG_FcnCode_t CommandCode;              /**< \brief Command code - identifies the command */
    uint16            Padding1;                 /**< \brief Structure padding to align to 32-bit boundaries */
    uint32            DirListOffset;            /**< \brief Starting entry for dir list commands */
    uint32            FileInfoState;            /**< \brief File info state */
    uint32            FileInfoSize;             /**< \brief File info size */
    uint32            FileInfoTime;             /**< \brief File info time */
    uint32            FileInfoCRC;              /**< \brief File info CRC method */
    char              Source1[OS_MAX_PATH_LEN]; /**< \brief First source file or directory name command argument */
    char              Source2[OS_MAX_PATH_LEN]; /**< \brief Second source filename command argument */
    char              Target[OS_MAX_PATH_LEN];  /**< \brief Target filename command argument */
    uint8             GetSizeTimeMode; /**< \brief Whether to invoke stat call for size and time (CPU intensive) */
    uint8             Padding2[3];     /**< \brief Structure padding to align to 32-bit boundaries */
    uint32            Mode;            /**< \brief File Mode */
} FM_ChildQueueEntry_t;

#endif