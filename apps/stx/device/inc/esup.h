#ifndef _ESUP_H_ 
#define _ESUP_H_

#include <stdio.h>
#include <inttypes.h>

/*
ESUP Packet
--------------------------------------------------------------------------------------------------
| Header | Module ID | Data Length | Command Status | Command | Type    | Data         | CRC32   |
| 4 bytes| 2 bytes   | 2 bytes     | 2 bytes        | 2 bytes | 2 bytes | 0-1472 bytes | 4 bytes |
--------------------------------------------------------------------------------------------------   
Overall Length must be a multiple of 16.
Data Length should be between 0 to 1472 bytes.
*/
// extern uint32_t open_file_size;

typedef struct {
    uint32_t header;
    uint16_t mod_id;
    uint16_t length;
    uint16_t com_stt;
    uint16_t command;
    uint16_t type;
}__attribute__((packed)) ESUP_Header_t;  // ESUP Packet에서 헤더 영역

typedef struct {
    ESUP_Header_t header;
    uint8_t DCP[];
}__attribute__((packed)) ESUP_Packet_t;  // ESUP Packet의 헤더와 데이터 영역

// RF parameters
typedef struct {
    unsigned char symbol_rate;     // Symbol Rate
    unsigned char tx_power;        // Transmit Power
    unsigned char modcod;          // MODCOD
    unsigned char roll_off;        // Roll-Off
    unsigned char pilot_signal;    // Pilot Signal
    unsigned char fec;             // FEC Frame Size
    unsigned short pre_delay;      // Pretransmission Delay
    float center_freq;             // Center Frequency
}__attribute__((packed)) RF_param_t;


#define ESUP_MAX_PACKET_LENGTH    1504        // (패딩포함) max data length 1490 아닌가? (그전에는 1472)
#define ESUP_MAX_DATA_LENGTH  1472
#define ESUP_MAX_WRITE_LENGTH  1200

#define ESUP_HEADER     0x50555345  // header
#define STX_MODULE_ID_A 0x1212
#define STX_MODULE_ID_B 0x1213
#define MODULE_ID       STX_MODULE_ID_A         // default module ID

// Command Status
#define ESUP_ACK        0x0005
#define ESUP_NACK       0x0006
#define ESUP_BUSY       0x0007
#define ESUP_NCE        0x0008
#define ESUP_FULL       0x0009
#define ESUP_TEMPN      0x0010
#define ESUP_INSIG      0x0000  // when the field is insignificant

// ESUP 함수 오류
#define STX_ESUP_ENCODER_ERR   -100
#define STX_ESUP_DECODER_ERR   -101
#define STX_ESUP_WRITE_ERR   -102
#define STX_ESUP_READ_ERR   -103
#define STX_ESUP_HEADF_ERR   -104
#define STX_ESUP_DATAS_ERR   -105
#define STX_ESUP_MODULE_ID_ERR -106
#define STX_ESUP_DATALENGTH_MISMATCH_ERR -107

// Command
#define CONFIG_CUSTOM_SCAN          0xffff

#define CONFIG_CC_GET               0x0100
#define CONFIG_CC_SET               0x0101

#define STATUS_CC_GET               0x0100

#define FILESYS_CC_DIR              0x0102
#define FILESYS_CC_DIRNEXT          0x0103
#define FILESYS_CC_DELFILE          0x0104
#define FILESYS_CC_DELALLFILE       0x0105
#define FILESYS_CC_CREATEFILE       0x0106
#define FILESYS_CC_WRITEFILE        0x0107
#define FILESYS_CC_OPENFILE         0x0108
#define FILESYS_CC_READFILE         0x0109
#define FILESYS_CC_SENDFILE         0x010A

#define SYSCONF_CC_TRANSMITMODE     0x0110
#define SYSCONF_CC_IDLEMODE         0x0111
#define SYSCONF_CC_UPDATEFW         0x0112
#define SYSCONF_CC_SAFESHUTDOWN     0x0113

#define GETRES_CC_GETRES            0x0114

// Type
#define CONFIG_TP_SYMBOLRATE        0x0040 
#define CONFIG_TP_TRANSMITPW        0x0041 
#define CONFIG_TP_CENTERFREQ        0x0042 
#define CONFIG_TP_MODCOD            0x0043 
#define CONFIG_TP_ROLLOFF           0x0044 
#define CONFIG_TP_PILOTSIG          0x0045 
#define CONFIG_TP_FECFRAMESZ        0x0046 
#define CONFIG_TP_PRETXSTUFFDEL     0x0047 
#define CONFIG_TP_ALLPARAM          0x0048
#define CONFIG_TP_RS485BAUD         0x004B
#define CONFIG_TP_MODULATORDTIFC    0x004D

#define STATUS_TP_SIMPLE_REPORT     0x0049
#define STATUS_TP_FULL_REPORT       0x004A

#define FILESYS_TP_NA               0x0000
#define FILESYS_TP_SENDFILE         0x0050
#define FILESYS_TP_SENDFILEPI       0x0051
#define FILESYS_TP_SENDFILERTI      0x0052
#define FILESYS_TP_SENDFILEAI       0x0053

#define SYSCONF_TP_NA               0x0000

#define GETRES_TP_NA                0X0000


int32_t ESUP(uint16_t comm_stt, uint16_t comm, uint16_t type, void * txdata, uint16_t txlength, ESUP_Packet_t * rxdata, uint16_t rxlength);
long latch_ms(void);
int32_t ESUP_ACK_CMD(uint16_t comm_stt, uint16_t comm, uint16_t type);
int32_t ESUP_SetModuleId(uint16_t module_id);
uint16_t ESUP_GetModuleId(void);

#endif
