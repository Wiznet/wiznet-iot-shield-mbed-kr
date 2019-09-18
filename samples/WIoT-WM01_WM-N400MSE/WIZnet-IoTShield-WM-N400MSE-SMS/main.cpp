/* WIZnet IoT Shield Cat.M1 Sample code for Arm MBED
 * Copyright (c) 2019 WIZnet Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 */
 
 /* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "mbed.h"

#include <string>

#define RET_OK                      1
#define RET_NOK                     -1
#define DEBUG_ENABLE                1
#define DEBUG_DISABLE               0
#define ON                          1
#define OFF                         0

#define MAX_BUF_SIZE                1024

#define WM01_APN_PROTOCOL_IPv4      1
#define WM01_APN_PROTOCOL_IPv6      2
#define WM01_DEFAULT_TIMEOUT        1000
#define WM01_CONNECT_TIMEOUT        15000
#define WM01_SEND_TIMEOUT           500
#define WM01_RECV_TIMEOUT           500

#define WM01_APN_PROTOCOL           WM01_APN_PROTOCOL_IPv6
#define WM01_DEFAULT_BAUD_RATE      115200
#define WM01_PARSER_DELIMITER       "\r\n"

#define CATM1_APN_SKT               "lte-internet.sktelecom.com"

#define CATM1_DEVICE_NAME_WM01      "WM01"
#define DEVNAME                     CATM1_DEVICE_NAME_WM01

#define devlog(f_, ...)             if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { pc.printf("\r\n[%s] ", DEVNAME);  pc.printf((f_), ##__VA_ARGS__); }
#define myprintf(f_, ...)           {pc.printf("\r\n[MAIN] ");  pc.printf((f_), ##__VA_ARGS__);}

/* Pin configuraiton */
// Cat.M1
#define MBED_CONF_IOTSHIELD_CATM1_TX        D8
#define MBED_CONF_IOTSHIELD_CATM1_RX        D2
#define MBED_CONF_IOTSHIELD_CATM1_RESET     D7
#define MBED_CONF_IOTSHIELD_CATM1_PWRKEY    D9

// Sensors
#define MBED_CONF_IOTSHIELD_SENSOR_CDS      A0
#define MBED_CONF_IOTSHIELD_SENSOR_TEMP     A1

/* Debug message settings */
#define WM01_PARSER_DEBUG                   DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG                  DEBUG_ENABLE 

/* SMS */
#define SMS_EOF                             0x1A
#define MAX_SMS_SIZE                        100

// have to modify phone number
char phone_number[] = "010xxxxxxxx";
char send_message[] = "WIZnet Cat.M1 IoT shield is powered on";

// Functions: Print information
void printInfo(void);

// Functions: Module Status
void waitCatM1Ready(void);
int8_t setEchoStatus_WM01(bool onoff);
int8_t getUsimStatus_WM01(void);
int8_t getNetworkStatus_WM01(void);

// Functions: SMS
int8_t initSMS_WM01(void);
int8_t sendSMS_WM01(char *da, char *msg, int len);
int checkRecvSMS_WM01(void);
int8_t recvSMS_WM01(int msg_idx, char *datetime, char *da, char *msg);
int8_t deleteSMS_WM01(int msg_idx);
int8_t deleteAllSMS_WM01(int delflag);

Serial pc(USBTX, USBRX);    // USB debug

UARTSerial *_serial;        // Cat.M1 module    
ATCmdParser *_parser;

DigitalOut _RESET_WM01(MBED_CONF_IOTSHIELD_CATM1_RESET);
DigitalOut _PWRKEY_WM01(MBED_CONF_IOTSHIELD_CATM1_PWRKEY);

void serialPcInit(void)
{
    pc.baud(115200);
    pc.format(8, Serial::None, 1);
}

void serialDeviceInit(PinName tx, PinName rx, int baudrate) 
{        
    _serial = new UARTSerial(tx, rx, baudrate);    
}

void serialAtParserInit(const char *delimiter, bool debug_en)
{
    _parser = new ATCmdParser(_serial);    
    _parser->debug_on(debug_en);
    _parser->set_delimiter(delimiter);    
    _parser->set_timeout(WM01_DEFAULT_TIMEOUT);
}

void catm1DeviceInit(void)
{
    serialDeviceInit(   MBED_CONF_IOTSHIELD_CATM1_TX, 
                        MBED_CONF_IOTSHIELD_CATM1_RX, 
                        WM01_DEFAULT_BAUD_RATE);
                        
    serialAtParserInit( WM01_PARSER_DELIMITER, 
                        WM01_PARSER_DEBUG);
}

void catm1DeviceReset_WM01(void)
{
    _RESET_WM01 = 1;
    _PWRKEY_WM01 = 1;
    wait_ms(300);
    
    _RESET_WM01 = 0;
    _PWRKEY_WM01 = 0;
    wait_ms(400);
    
    _RESET_WM01 = 1;    
    wait_ms(1000);
}

// ----------------------------------------------------------------
// Main routine
// ----------------------------------------------------------------

int main()
{
    serialPcInit();    
    catm1DeviceInit();
    
    myprintf("Waiting for Cat.M1 Module Ready...\r\n");
    
    catm1DeviceReset_WM01();
    
    waitCatM1Ready();
    
    wait_ms(5000);
            
    myprintf("System Init Complete\r\n");

    printInfo();
    
    setEchoStatus_WM01(OFF);
   
    getUsimStatus_WM01();
    
    getNetworkStatus_WM01();
    
    // SMS configuration
    if(initSMS_WM01() != RET_OK) 
    {   
        myprintf("[SMS Init] failed\r\n");
    }
    
    // Send a message 
    if(sendSMS_WM01(phone_number, send_message, strlen(send_message)) == RET_OK) 
    {
        myprintf("[SMS Send] to %s, \"%s\"\r\n", phone_number, send_message);
    }
    
#if 0
// Delete messages
    deleteAllSMS_WM01(3);
#endif

    char date_time[25] = {0, };
    char dest_addr[20] = {0, };
    char recv_message[MAX_SMS_SIZE] = {0, };
    int msg_idx;
    
    while(1)
    {        
        // SMS receive check
        msg_idx = checkRecvSMS_WM01();
        
        if(msg_idx > RET_NOK)   // SMS received
        {   
            // Receive a message
            memset(recv_message, 0x00, MAX_SMS_SIZE);

            if(recvSMS_WM01(msg_idx, date_time, dest_addr, recv_message) == RET_OK) 
            {
                myprintf("[SMS Recv] from %s, %s, \"%s\"\r\n", dest_addr, date_time, recv_message);
            }
        }     
    }
}

// ----------------------------------------------------------------
// Functions: Print information
// ----------------------------------------------------------------

void printInfo(void)
{
    myprintf("WIZnet IoT Shield for Arm MBED");
    myprintf("LTE Cat.M1 Version");
    myprintf("=================================================");
    myprintf(">> Target Board: WIoT-WM01 (Woorinet WM-N400MSE)");
    myprintf(">> Sample Code: SMS Test");
    myprintf("=================================================\r\n");
}

// ----------------------------------------------------------------
// Functions: Cat.M1 Status
// ----------------------------------------------------------------

void waitCatM1Ready(void)
{
    while(1) 
    {
        if(_parser->send("AT") && _parser->recv("OK"))
        {
            myprintf("WM01 is Available\r\n");
            
            return;
        }
    }
}

int8_t setEchoStatus_WM01(bool onoff)
{
    int8_t ret = RET_NOK;
    char _buf[10];
    
    sprintf((char *)_buf, "ATE%d", onoff);    
    
    if(_parser->send(_buf) && _parser->recv("OK")) 
    {        
        devlog("Turn Echo %s : success\r\n", onoff ? "ON" : "OFF");

        ret = RET_OK;
    } 
    else 
    { 
        devlog("Turn Echo %s : failed\r\n", onoff ? "ON" : "OFF");
    }    

    return ret;
}
 
int8_t getUsimStatus_WM01(void)
{
    int8_t ret = RET_NOK;
    
    if(_parser->send("AT$$STAT?") && _parser->recv("$$STAT:READY") && _parser->recv("OK")) 
    {
        devlog("USIM Status : READY\r\n");

        ret = RET_OK;
    } 
    else 
    { 
        devlog("Retrieving USIM Status failed\r\n");        
    }
    
    return ret;
}

int8_t getNetworkStatus_WM01(void)
{
    int8_t ret = RET_NOK;
    int val, stat;
    
    if(_parser->send("AT+CEREG?") && _parser->recv("+CEREG: %d,%d", &val, &stat) && _parser->recv("OK")) 
    {
        if((val == 0) && (stat == 1))
        {
            devlog("Network Status : attached\r\n");

            ret = RET_OK;
        }
        else
        {
            devlog("Network Status : %d, %d\r\n", val, stat);
        }
    }
    else
    {
        devlog("Network Status : Error\r\n");
    }

    return ret;
}

// ----------------------------------------------------------------
// Functions: Cat.M1 SMS
// ----------------------------------------------------------------

int8_t initSMS_WM01(void)
{
    int8_t ret = RET_NOK;
    bool msgformat, charset, recvset = false;
    
    // 0 = PDU mode / 1 = Text mode
    if(_parser->send("AT+CMGF=1") && _parser->recv("OK"))       // Set SMS message format as text mode
    {         
        devlog("SMS message format : Text mode\r\n");

        msgformat = true;
    }    
    
    // "GSM" / "IRA" / "USC2"
    if(_parser->send("AT+CSCS=\"GSM\"") && _parser->recv("OK")) // Set character set as GSM
    {
        devlog("SMS character set : GSM\r\n");

        charset = true;
    }

    if(_parser->send("AT*SKT*NEWMSG=4098") && _parser->recv("OK"))
    {
        devlog("Tele service ID set : 4098\r\n");

        recvset = true;
    }
    
    if(msgformat && charset && recvset) 
    {
        ret = RET_OK;
    }
    
    return ret;    
}

int8_t sendSMS_WM01(char *da, char *msg, int len)
{
    int8_t ret = RET_NOK;
    bool done = false;
    int msg_idx = 0;    

    _parser->set_timeout(WM01_CONNECT_TIMEOUT);
    
    _parser->send("AT+CMGS=\"%s\"", da);    // DA(Destination address, Phone number)

    if(!done && _parser->recv(">"))
    {
        done = (_parser->write(msg, len) <= 0) & _parser->send("%c", SMS_EOF);
    }
        
    if(!done) 
    {        
        done = (_parser->recv("+CMGS: %d", &msg_idx) && _parser->recv("OK"));        
        
        if(done)
        {
            devlog(">> SMS send success : index %d\r\n", msg_idx);

            ret = RET_OK;
        }
    }

    _parser->set_timeout(WM01_DEFAULT_TIMEOUT);
    
    return ret;
}

int checkRecvSMS_WM01(void)
{  
    bool received = false;  
    int ret = RET_NOK;
    int msg_idx = 0;
    int tele_service_id = 0;    
    
    _parser->set_timeout(1);

    received = _parser->recv("*SKT*NEWMSG:%d", &msg_idx);

    _parser->set_timeout(WM01_DEFAULT_TIMEOUT);
    
    if(received)
    {        
        ret = msg_idx; 
        
        devlog("<< SMS received : index %d\r\n", msg_idx);
    }

    return ret;
}

int8_t recvSMS_WM01(int msg_idx, char *datetime, char *da, char *msg)
{
    int8_t ret = RET_NOK;    
    bool done = false;  
    char type[15] = {0, };
    char recv_msg[MAX_SMS_SIZE] = {0, };
    char *search_pt;    
    int i = 0;
    
    Timer t;
    
    memset(recv_msg, 0x00, MAX_SMS_SIZE);
        
    _parser->set_timeout(WM01_RECV_TIMEOUT);   
    
    if(_parser->send("AT+CMGR=%d", msg_idx) && _parser->recv("+CMGR: \"%[^\"]\",\"%[^\"]\",,\"%[^\"]\"", type, da, datetime))
    {        
        // timer start
        t.start();
        
        while(!done && (t.read_ms() < WM01_DEFAULT_TIMEOUT)) 
        {        
            _parser->read(&recv_msg[i++], 1);

            search_pt = strstr(recv_msg, "OK");

            if(search_pt != 0) 
            {
                done = true;    // break;
            }
        }   

        if(i > 8) 
        {
            memcpy(msg, recv_msg + 2, i - 8);            
            devlog("<< SMS receive success : index %d\r\n", msg_idx);  

            ret = RET_OK;
        }
    }

    _parser->set_timeout(WM01_DEFAULT_TIMEOUT);    
    _parser->flush();
            
    return ret;
}

int8_t deleteSMS_WM01(int msg_idx)
{
    int8_t ret = RET_NOK;
    
    if(_parser->send("AT+CMGD=%d", msg_idx) && _parser->recv("OK"))
    {
        devlog("Message index[%d] has been deleted\r\n", msg_idx);           

        ret = RET_OK;
    } 
    return ret;
}

int8_t deleteAllSMS_WM01(int delflag)
{
    int8_t ret = RET_NOK;    
    
    // delflag == 1;    // Delete all read messages from storage
    // delflag == 2;    // Delete all read messages from storage and sent mobile originated messages
    // delflag == 3;    // Delete all read messages from storage, sent and unsent mobile originated messages
    // delflag == 4;    // Delete all messages from storage
    if(_parser->send("AT+CMGD=0,%d", delflag) && _parser->recv("OK"))
    {
        devlog("All messages has been deleted (delflag : %d)\r\n", delflag);

        ret = RET_OK;
    } 
    
    return ret;
}