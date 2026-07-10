#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <esup.h>
#include <crc32.h>
#include <byteswap.h>
#include "cfe.h"
#include "cfe_srl.h"
#include <time.h>
#include "stx_eventids.h"

extern CFE_SRL_IO_Handle_t *Handle;
uint32_t open_file_size = 0;
CFE_SRL_IO_Param_t Params = {0,};
uint16_t STX_timeout = 30; //[ms]
static uint16_t ESUP_ModuleId = MODULE_ID;

int32_t ESUP_SetModuleId(uint16_t module_id)
{
    ESUP_ModuleId = module_id;
    return 0;
}

uint16_t ESUP_GetModuleId(void)
{
    return ESUP_ModuleId;
}

static uint16_t ESUP_Encoder(uint16_t comm_stt, uint16_t comm, uint16_t type, void * data, uint16_t length, uint16_t padlen, ESUP_Packet_t * packet)
{   
    if(!packet)
        return 0;

    packet->header.header = ESUP_HEADER;                              // Header
    packet->header.mod_id = ESUP_ModuleId;                            // Module ID
    packet->header.length = length;                                   // Data Length
    packet->header.com_stt = comm_stt;                                // Command Status
    packet->header.command = comm;                                    // Command
    packet->header.type = type;                                       // Type
    memcpy(packet->DCP, data, length);                                // 인자로 받은 data를 ESUP packet의 데이터 영역인 DCP로 data length만큼 복사
    uint32_t c32 = crc32(0, packet, sizeof(ESUP_Header_t) + length);  // CRC32 계산
    memcpy(packet->DCP + length, &c32, sizeof(c32));                  // CRC32 bit 추가
    memset(packet->DCP + length + sizeof(c32), 0, padlen);            // zero padding
    // ESUP packet 완성

    return length + padlen + sizeof(c32) + sizeof(ESUP_Header_t);                   // ESUP packet의 전체 길이 return
    
}

static uint16_t ESUP_Decoder(ESUP_Packet_t * packet)
{   
    if(!packet)
        return 0;

    char headerbuf[5];                          // ESUP packet의 header 출력 공간
    memset(headerbuf, 0, sizeof(headerbuf));    // 0으로 초기화
    memcpy(headerbuf, &packet->header.header, sizeof(uint32_t));  // 읽은 데이터의 초기 4바이트를 headerbuf에 복사
    headerbuf[4] = 0;                           // 문자열의 마지막은 null

    // ESUP packet 정보 출력
    OS_printf("----ESUP Decoder----\n");
    OS_printf("Header         : %s\n", headerbuf);                  // Header: ESUP in ASCII(0x55055345 in HEX)
    OS_printf("Module ID      : %hx\n", packet->header.mod_id);     // Module ID
    OS_printf("Data Length    : %04hx\n", packet->header.length);     // Data Length
    OS_printf("Command Status : %04hx\n", packet->header.com_stt);   // Command Status
    OS_printf("Command        : %04hx\n", packet->header.command);  // Command
    OS_printf("Type           : %04hx\n", packet->header.type);     // Type
    OS_printf("Data           : \n");                               // Data

    for(int i = 0; i < packet->header.length; i++)
    {
        OS_printf("%02hhx\t", packet->DCP[i]);  // 1 바이트씩 unsigned char 형태로 출력
        if(i % 10 == 9)
            OS_printf("\n");
    }

    OS_printf("\n");

    return packet->header.mod_id;
}

long latch_ms(void) {
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
		return -1;
	}
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int32_t ESUP_Receive(ESUP_Packet_t* packet, uint16_t timeout) {

    int ret;
    unsigned headerIndex = 0;
    uint32_t header = ESUP_HEADER;
    size_t packetSize;
    uint8_t byte;

    long t1;

    t1 = latch_ms();

    /**
     * Search for the header sequence.
     */
    while (latch_ms() - t1 < timeout) {


        Params.TxData = NULL;
        Params.TxSize = 0;
        Params.RxData = &byte;
        Params.RxSize = 1;
        Params.Timeout = STX_timeout;
        ret = CFE_SRL_ApiRead(Handle, &Params);

        if (ret < 0) {
            /**
             * Read error or timeout reached.
             */
            return ret;
        }

        if (byte == ((const uint8_t*) &header)[headerIndex]) {
            headerIndex++;
            if (headerIndex == sizeof(header)) {
                /**
                 * Header found.
                 */
                break;
            }
        }
        else {
            headerIndex = 0;
        }
    }

    if (headerIndex < sizeof(header)) {
        /**
         * No header found until timeout expired.
         */
        OS_printf("fail to find header \n");
        return STX_ESUP_HEADF_ERR;
    }
    memcpy(packet, &header, sizeof(header));
    /**
     * Here we have a valid header. Read the rest of the packet,
     * except the data part.
     */
    Params.TxData = NULL;
    Params.TxSize = 0;
    Params.RxData = ((uint8_t*) packet) + sizeof(header);
    Params.RxSize =  sizeof(*packet) - sizeof(header);
    Params.Timeout = STX_timeout;
    ret = CFE_SRL_ApiRead(Handle, &Params);
    
    if (ret < 0) {
        return ret;
    }
    OS_printf("length: %u \n",packet->header.length);
    if (packet->header.length > ESUP_MAX_DATA_LENGTH) {
        
        return STX_ESUP_DATAS_ERR;
    }

    /**
     * Now read the full data.
     */
    // packetSize = sizeof(*packet) + packet->header.length;
    packetSize = packet->header.length + 4;
    packetSize += ((sizeof(ESUP_Header_t) + packetSize) % 16) ? 16 - ((sizeof(ESUP_Header_t)+ packetSize) % 16) : 0;
    
    if (packetSize > ESUP_MAX_PACKET_LENGTH) {
        return STX_ESUP_DATAS_ERR;
    }

    
    Params.TxData = NULL;
    Params.TxSize = 0;
    Params.RxData = (uint8_t*)packet + sizeof(ESUP_Header_t);
    Params.RxSize =  packetSize;
    Params.Timeout = STX_timeout;
    ret = CFE_SRL_ApiRead(Handle, &Params);

    if (ret < 0) {
        return ret;
    }
    return 0;
}


int32_t ESUP_ACK_CMD(uint16_t comm_stt, uint16_t comm, uint16_t type)
{
    uint16_t retu_len = 0;
    int32_t retu_status = 0;

    uint16_t lencal = sizeof(ESUP_Header_t) + 4;
    int padlen = (lencal % 16) ? (16 - lencal % 16) : 0;            
    lencal += padlen;

    char temppacket[lencal];
    memset(temppacket, 0, sizeof(temppacket));
    ESUP_Packet_t * packet = (ESUP_Packet_t *)temppacket;

    OS_printf("Write Bytes : %u\n", lencal);

    retu_len = ESUP_Encoder(comm_stt, comm, type, NULL, 0, padlen, packet);
    
    if(retu_len <= 0)  // retu16: 생성한 ESUP packet의 전체 길이
    {
        OS_printf("ESUP Write Failed!\n");
        return STX_ESUP_ENCODER_ERR;
    }

    Params.TxData = packet;
    Params.TxSize = lencal;
    retu_status = CFE_SRL_ApiWrite(Handle, &Params);
    if(retu_status < 0)
    {
        OS_printf("SRL_ApiWrite failed \n");
        OS_printf("RS485 has no reply.\n");
        OS_printf("%d\n", retu_status); 
        return STX_ESUP_WRITE_ERR;
    }

    return retu_status;
}

int32_t ESUP(uint16_t comm_stt, uint16_t comm, uint16_t type, void * txdata, uint16_t txlength, ESUP_Packet_t * rxdata, uint16_t rxlength)
{   
    uint16_t retu_len = 0;
    int32_t retu_status = 0;

    uint16_t lencal = sizeof(ESUP_Header_t) + txlength + 4;
    int padlen = (lencal % 16) ? (16 - lencal % 16) : 0;            
    lencal += padlen;

    char temppacket[lencal];
    memset(temppacket, 0, sizeof(temppacket));
    ESUP_Packet_t * packet = (ESUP_Packet_t *)temppacket;  
    
    uint16_t rx_lencal = sizeof(ESUP_Header_t) + rxlength + 4;
    int rpadlen = 16 - rx_lencal % 16;            
    rx_lencal += rpadlen;

    char readbuf[rx_lencal];           
    memset(readbuf, 0, sizeof(readbuf));  
    ESUP_Packet_t *rxbuf = (ESUP_Packet_t *)readbuf;
  
    OS_printf("Write data Bytes: %u\n", txlength);
    OS_printf("Write Bytes : %u\n", lencal);
    OS_printf("Read Bytes : %d\n", rx_lencal);


    retu_len = ESUP_Encoder(comm_stt, comm, type, txdata, txlength, padlen, packet);
    
    if(retu_len <= 0)  // retu16: 생성한 ESUP packet의 전체 길이
    {
        OS_printf("ESUP Write Failed!\n");
        return STX_ESUP_ENCODER_ERR;
    }

    // read
    Params.TxData = packet;
    Params.TxSize = lencal;
    retu_status = CFE_SRL_ApiWrite(Handle, &Params);
    if(retu_status < 0)
    {
        OS_printf("SRL_ApiWrite failed \n");
        OS_printf("RS485 has no reply.\n");
        OS_printf("%d\n", retu_status); 
        return STX_ESUP_WRITE_ERR;
    }
    
    usleep(50000); //[micro s] 

    retu_status = ESUP_Receive(rxbuf, STX_timeout);

    if(retu_status < 0)
    {
        OS_printf("ESUP receive fail\n");
        OS_printf("RS485 has no reply.\n");
        OS_printf("%d\n", retu_status); 
        return STX_ESUP_READ_ERR;
    }

    memcpy(rxdata, rxbuf, sizeof(ESUP_Packet_t) + rxbuf->header.length);

    retu_len = ESUP_Decoder(rxdata);
    if(retu_len <= 0)
    {
        OS_printf("ESUP Read Failed!\n");
        return STX_ESUP_DECODER_ERR;
    }
    if(retu_len != ESUP_ModuleId)
    {
        OS_printf("ESUP module id mismatch. expected=%04hx received=%04hx\n", ESUP_ModuleId, retu_len);
        return STX_ESUP_MODULE_ID_ERR;
    }
    /********************************************************************************* */
    if (rxdata->header.command == 0x0108 && rxdata->header.length != 0x0000 && rxdata->DCP[0] == 0x00){
        memcpy(&open_file_size, &(rxdata->DCP[5]), sizeof(uint32_t));
        OS_printf("open file size : %d\n", open_file_size);
    }
    /********************************************************************************* */

    if (rxdata->header.length != rxlength){
        OS_printf("data length mismatch \n");
    }

    OS_printf("ESUP_read success via RS485.\n");

    return retu_status;
}
