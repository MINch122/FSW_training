// Last Modified Date 2025.01.24

#include <stdio.h>
#include <stdint.h>
#include "rs485.h"
#include "esup.h"

int main22(void)
{
    int ret = 0;
    
    uint16_t status = 0;   // Command Status 
    uint16_t command = 1;  // Command
    uint16_t type = 0;     // Type

    RF_param_t rf_param;            // ESUP packet의 data 영역에 들어가는 RF parameters
    RF_param_t *param = &rf_param;  // RF parameter 구조체를 가리키는 포인터

    uint16_t getresult = 0;         // GetResult command에 입력할 data
    uint8_t rs485 = 0;              // RS485 baud rate

    char buf[1472];          // Data
    char input[1472];        // Input Data, buf로 전달
    int length = 0;        // Data Length

    
    if((ret = RS485_INIT("/dev/ttyUSB0", 250000)) < 0)  // RS485 initialization
    {
        printf("RS485 Initilize Failed! %d\n", ret);
        return 0;
    }
    printf("RS485 Initilize Success.\n");

    while(command)
    {   
        printf("<Command Status>: \n");
        scanf("%hx", &status);
        
        // Start of Command Status ESUP_INSIG //
        if (status == ESUP_INSIG) {
            printf("ESUP_INSIG\n");

            printf("<Command>       : \n");
            scanf("%hx", &command);
            if(command == 0)
                break;
        
            switch(command) {

            // Start of GET //
            case CONFIG_CC_GET: {  // GET
                printf("GET\n");
                printf("<Type>          : \n");
                scanf("%hx", &type);
                ESUP(status, command, type, NULL, 0);  // GET
                usleep(30000);  // 30000us = 30ms
                ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult
                break;
            }
            // END of GET //

            // Start of SET //
            case CONFIG_CC_SET: {  // SET
                printf("SET\n");
                printf("<Type>          : \n");
                scanf("%hx", &type);
                
                switch(type) {
                case CONFIG_TP_SYMBOLRATE:  // Symbol Rate
                    printf("Symbol Rate\n");
                    printf("<Data>          : \n");
                    scanf("%hhu", &param->symbol_rate);
                    ESUP(status, command, type, &param->symbol_rate, sizeof(param->symbol_rate));  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult
                    break;
                    
                case CONFIG_TP_TRANSMITPW:  // Transmit Power
                    printf("Transmit Power\n");
                    printf("<Data>          : \n");
                    scanf("%hhu", &param->tx_power);
                    ESUP(status, command, type, &param->tx_power, sizeof(param->tx_power));  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult
                    break;
                    
                case CONFIG_TP_CENTERFREQ:  // Center Frequency
                    printf("Center Frequency\n");
                    printf("<Data>          : \n");
                    scanf("%f", &param->center_freq);
                    ESUP(status, command, type, &param->center_freq, sizeof(param->center_freq));  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult 
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult 
                    break;
                    
                case CONFIG_TP_MODCOD:  // MODCOD
                    printf("MODCOD\n");
                    printf("<Data>          : \n");
                    scanf("%hhu", &param->modcod);
                    ESUP(status, command, type, &param->modcod, sizeof(param->modcod));  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult 
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET 
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult
                    break;
                    
                case CONFIG_TP_ROLLOFF: // Roll-Off
                    printf("Roll-Off\n");
                    printf("<Data>          : \n");
                    scanf("%hhu", &param->roll_off);
                    ESUP(status,command, type, &param->roll_off, sizeof(param->roll_off));  // SET 
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult 
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET 
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult 
                    break;

                case CONFIG_TP_PILOTSIG: // Pilot Signal
                    printf("Pilot Signal\n");
                    printf("<Data>          : \n");
                    scanf("%hhu", &param->pilot_signal);
                    ESUP(status, command, type, &param->pilot_signal, sizeof(param->pilot_signal));  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult 
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult 
                    break;

                case CONFIG_TP_FECFRAMESZ: // FEC Frame Size
                    printf("FEC Frame Size\n");
                    printf("<Data>          : \n");
                    scanf("%hhu", &param->fec);
                    ESUP(status, command, type, &param->fec, sizeof(param->fec));  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult 
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult 
                    break;

                case CONFIG_TP_PRETXSTUFFDEL: // Pretransmission Delay
                    printf("Pretransmission Delay\n");
                    printf("<Data>          : \n");
                    scanf("%hu", &param->pre_delay);
                    ESUP(status, command, type, &param->pre_delay, sizeof(param->pre_delay));  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult 
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult 
                    break;

                case CONFIG_TP_ALLPARAM: // All Transmission Parameters
                    printf("All parameters\n");
                    printf("<Data>          : \n");
                    printf("Symbol Rate : ");
                    scanf("%hhu", &param->symbol_rate);
                    printf("Transmit Power : ");
                    scanf("%hhu", &param->tx_power);
                    printf("MODCOD : ");
                    scanf("%hhu", &param->modcod);
                    printf("Roll-Off : ");
                    scanf("%hhu", &param->roll_off);
                    printf("Pilot Signal : ");
                    scanf("%hhu", &param->pilot_signal);
                    printf("FEC Frame Size : ");
                    scanf("%hhu", &param->fec);
                    printf("Pretransmission Delay : ");  
                    scanf("%hu", &param->pre_delay);
                    printf("Center Frequency : ");
                    scanf("%f", &param->center_freq);
                    ESUP(status, command, type, &rf_param, sizeof(rf_param));  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult 
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult 
                    break;

                case CONFIG_TP_RS485BAUD: // RS485 Baud Rate
                    printf("RS485 Baud Rate\n");
                    printf("<Data>          : \n");
                    scanf("%hhu", &rs485);
                    ESUP(status, command, type, &rs485, sizeof(rs485));  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult 
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult
                    break;

                case CONFIG_TP_MODULATORDTIFC: // Modulator Data Interface
                    printf("Change RS485 Baud Rate\n");
                    printf("<Data Length>          : \n");
                    length = 2;
                    for(int i = 0; i < length; i++)
                    {
                        printf("<Data_%d>       : \n", i);
                        scanf("%hhx", &buf[i]);
                    }
                    ESUP(status, command, type, buf, length);  // SET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult
                    usleep(30000);
                    ESUP(status, CONFIG_CC_GET, type, NULL, 0);  // GET
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, CONFIG_CC_GET, &type, sizeof(type));  // GetResult
                    break;
                }
                break;
            }
            // End of SET //

            // Start of DIR //
            case FILESYS_CC_DIR: {
                printf("DIR\n");
                ESUP(status, command, FILESYS_TP_NA, NULL, 0);  // DIR
                usleep(30000);  // 30000us = 30ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of DIR //

            // Start of DIR Next //
            case FILESYS_CC_DIRNEXT: {
                printf("DIR Next\n");
                ESUP(status, command, FILESYS_TP_NA, NULL, 0);  // DIR NEXT
                usleep(30000);  // 30000us = 30ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of DIR Next //

            // Start of Delete File //
            case FILESYS_CC_DELFILE: {
                printf("Delete File\n");
                
                printf("<Data Length>          : \n");
                scanf("%d", &length);
                for(int i = 0; i < length; i++)
                {
                    printf("<Data_%d>       : \n", i);
                    scanf("%hhx", &buf[i]);
                }
                
                /*
                printf("Enter a file name: ");
                fgets(input, sizeof(input), stdin);
                length = strlen(input);
                if (input[length - 1] == '\n') {
                    input[length - 1] = '\0';
                    length--;
                }
                for (int i = 0; i < length; i++) {
                    buf[i] = input[i];
                }
                buf[length] = '\0';
                length = length + 1;
                */

                ESUP(status, command, FILESYS_TP_NA, buf, length);  // Delete File
                usleep(30000);  // 30000us = 30ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of Delete File //

            // Start of Delete All Files //
            case FILESYS_CC_DELALLFILE: {
                printf("Delete All Files\n");
                ESUP(status, command, FILESYS_TP_NA, NULL, 0);  // Delete All Files
                usleep(130000);  // 130000us = 130ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of Delete All File //

            // Start of Create File //
            /*
                Create File & GetResult를 수행하면 그에 대한 응답으로 Status와 Packet Handler 출력.
                그리고 이 Packet Handler를 사용해서 Write File를 수행해야 되는데 이때 endian이 중요.
                자세한 내용은 Write File section 확인.
            */
            case FILESYS_CC_CREATEFILE: {
                printf("Create File\n");
                printf("<Data Length>          : \n");
                scanf("%d", &length);
                for(int i = 0; i < length; i++)
                {
                    printf("<Data_%d>       : \n", i);
                    scanf("%hhx", &buf[i]);
                }
                ESUP(status, command, FILESYS_TP_NA, buf, length);  // Create File
                usleep(30000);  // 30000us = 30ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of Create File //

            // Start of Write File //
            /*
                Create File & GetResult를 통해 얻은 Packet Handler를 데이터 영역에 적어야 됨.
                이때 data 영역에서 Data Packet Length는 데이터시트와 반대 endian으로 입력.
                Packet Handler는 시스템 상에서 출력된 그대로 (endian 유지) 입력.
                이와 같은 내용은 Open File, Read File에도 동일하게 적용됨.
            */
            case FILESYS_CC_WRITEFILE: {
                printf("Write File\n");
                printf("<Data Length>          : \n");
                scanf("%d", &length);
                for(int i = 0; i < length; i++)
                {
                    printf("<Data_%d>       : \n", i);
                    scanf("%hhx", &buf[i]);
                }
                ESUP(status, command, FILESYS_TP_NA, buf, length);  // Write File
                usleep(30000);  // 30000us = 30ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of Write File //

            // Start of Open File //
            case FILESYS_CC_OPENFILE: {
                printf("Open File\n");
                printf("<Data Length>          : \n");
                scanf("%d", &length);
                for(int i = 0; i < length; i++)
                {
                    printf("<Data_%d>       : \n", i);
                    scanf("%hhx", &buf[i]);
                }
                ESUP(status, command, FILESYS_TP_NA, buf, length);  // Open File
                usleep(30000);  // 30000us = 30ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of Open File //

            // Start of Read File //
            case FILESYS_CC_READFILE: {
                printf("Read File\n");
                printf("<Data Length>          : \n");
                scanf("%d", &length);
                for(int i = 0; i < length; i++)
                {
                    printf("<Data_%d>       : \n", i);
                    scanf("%hhx", &buf[i]);
                }
                ESUP(status, command, FILESYS_TP_NA, buf, length);  // Read File
                usleep(30000);  // 30000us = 30ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of Read File //

            // Start of Send File/s //
            /*
                Send File/s을 하면 command에 대한 응답은 잘 출력되는데 spectrum 분석기에 파형이 찍히지 않는 문제 발생.
                Spectrum 분석기가 packet 하나에 대해서는 제대로 파형을 못 찍는 것 같음.
                따라서 spectrum 분석기에 파형이 찍히게 하기 위해서는 Send File/s에 대한 command에 반복문을 추가해서 계속해서 packet을 보낼 수 있도록 코드를 수정해야 될 것 같음.
            */
            case FILESYS_CC_SENDFILE: {
                printf("Send File/s\n");
                printf("<Type>          : \n");
                scanf("%hx", &type);
                
                switch(type) {
                case FILESYS_TP_SENDFILE:  // Send File(s)
                    printf("Send File(s)\n");
                    printf("<Data Length>          : \n");
                    scanf("%d", &length);
                    for(int i = 0; i < length; i++)
                    {
                        printf("<Data_%d>       : \n", i);
                        scanf("%hhx", &buf[i]);
                    }

                    for(int i = 0; i < 20; i++){
                        ESUP(status, command, type, buf, length);  // Send File/s
                        usleep(30000);  // 30000us = 30ms
                        ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult
                        usleep(2000000);  // 2000000us = 1s
                    }
                    //ESUP(status, command, type, buf, length);  // Send File/s
                    //usleep(30000);  // 30000us = 30ms
                    //ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult
                    break;
                    
                case FILESYS_TP_SENDFILEPI:  // Send File(s) with Predistortion issue
                    printf("Send File(s) with Predistortion issue\n");
                    printf("<Data Length>          : \n");
                    scanf("%d", &length);
                    for(int i = 0; i < length; i++)
                    {
                        printf("<Data_%d>       : \n", i);
                        scanf("%hhx", &buf[i]);
                    }
                    ESUP(status, command, type, buf, length);  // Send File/s
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult
                    break;

                case FILESYS_TP_SENDFILERTI:  // Send File(s) with RF Tract issue
                    printf("Send File(s) with RF Tract issue\n");
                    printf("<Data Length>          : \n");
                    scanf("%d", &length);
                    for(int i = 0; i < length; i++)
                    {
                        printf("<Data_%d>       : \n", i);
                        scanf("%hhx", &buf[i]);
                    }
                    ESUP(status, command, type, buf, length);  // Send File/s
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult
                    break;

                case FILESYS_TP_SENDFILEAI:  // Send File(s) with all issues
                    printf("Send File(s) with all issues\n");
                    printf("<Data Length>          : \n");
                    scanf("%d", &length);
                    for(int i = 0; i < length; i++)
                    {
                        printf("<Data_%d>       : \n", i);
                        scanf("%hhx", &buf[i]);
                    }
                    ESUP(status, command, type, buf, length);  // Send File/s
                    usleep(30000);  // 30000us = 30ms
                    ESUP(status, GETRES_CC_GETRES, command, &type, sizeof(type));  // GetResult
                    break;
                }
                break;
            }
            // End of Send File/s //

            // Start of Transmit Mode //
            case SYSCONF_CC_TRANSMITMODE: {
                printf("Transmit Mode\n");
                ESUP(status, command, SYSCONF_TP_NA, NULL, 0);  // Transmit Mode
                usleep(40000000);  // 40000000us = 40000ms = 40s
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of Transmit Mode //

            // Start of Idle Mode //
            case SYSCONF_CC_IDLEMODE: {
                printf("Idle Mode\n");
                ESUP(status, command, SYSCONF_TP_NA, NULL, 0);  // Idle Mode
                usleep(50000);  // 50000us = 50ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of Idle Mode //

            // Start of Update Firmware //
            case SYSCONF_CC_UPDATEFW: {
                printf("Update Firmware\n");
                
                break;
            }
            // End of Update Firmware //

            // Start of Safe Shutdown //
            case SYSCONF_CC_SAFESHUTDOWN: {
                printf("Safe Shutdown\n");
                ESUP(status, command, SYSCONF_TP_NA, NULL, 0);  // Safe Shutdown
                usleep(630000);  // 600000us = 630ms
                ESUP(status, GETRES_CC_GETRES, command, NULL, 0);  // GetResult
                break;
            }
            // End of Safe Shutdown //

            // Start of GetResult //
            case GETRES_CC_GETRES: {
                printf("GetResult \n");
                printf("<Type>          : \n");
                scanf("%hx", &type);
                printf("<Data>          : \n");
                scanf("%hx", &getresult);
                if (getresult == 0) {  // GetResult command에 따른 data 영역에 값이 들어가지 않는 경우 0을 입력
                    ESUP(status, command, type, NULL, 0);
                }
                else {
                    ESUP(status, command, type, &getresult, sizeof(getresult));
                }
                break;
            }
            // End of GetResult //

            default: 
                printf("Wrong Command!\n");
                break;
            }
        }
        // End of Command Status ESUP_INSIG //

        // Start of Command Status ESUP_ACK //
        /*
            일단 Send ACKNOWLEDGE 코드가 아래와 같이 짜여져있는데 command flow를 보면 command->GetResult->Send ACKNOWLDGE 순서임.
            아래와 같은 코드로 Send ACKNOWLEDGE 했을 때 이상이 있었는데 만약 이 코드로 해결이 안된다면 command flow에 따라서 코드 수정이 필요해보임.
            command->GetResult 후 바로 밑에 Sent ACKNOWLEDGE 코드 한줄을 따로 추가해야 될 것 같음.
        */
        else if (status == ESUP_ACK) {
            printf("Send ACKNOWLEDGE\n");
            printf("<Command>       : \n");
            scanf("%hx", &command);
            printf("<Type>          : \n");
            scanf("%hx", &type);
            ESUP(status, command, type, NULL, 0);  // Sent ACK
            break;
        }
        // End of Command Status ESUP_ACK

        else {
            printf("Wrong Command Status!\n");
        }
        

        
    }

    return 0;    
}