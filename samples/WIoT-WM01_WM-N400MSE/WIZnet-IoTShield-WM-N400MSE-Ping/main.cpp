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

// Functions: Print information
void printInfo(void);

// Functions: Module Status
void waitCatM1Ready(void);
int8_t setEchoStatus_WM01(bool onoff);
int8_t getUsimStatus_WM01(void);
int8_t getNetworkStatus_WM01(void);

// Functions: PDP context
int8_t setContextActivate_WM01(void);   // Activate a PDP Context
int8_t setContextDeactivate_WM01(void); // Deactivate a PDP Context

// Functions: Ping test
void printPingToHost_WM01(char *host, int pingnum);

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
    char ping_dest_1st[] = "8.8.8.8";
    char ping_dest_2nd[] = "www.google.com"; 

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

    setContextActivate_WM01();

    myprintf("[Ping] Host : %s\r\n", ping_dest_1st);
    printPingToHost_WM01(ping_dest_1st, 4); 
    
    wait_ms(2000);
    
    myprintf("[Ping] Host : %s\r\n", ping_dest_2nd);
    printPingToHost_WM01(ping_dest_2nd, 4);
        
    setContextDeactivate_WM01(); 
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
    myprintf(">> Sample Code: Ping Test");
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
        devlog("Turn Echo %s : success\r\n", onoff?"ON":"OFF");

        ret = RET_OK;
    } 
    else 
    { 
        devlog("Turn Echo %s : failed\r\n", onoff?"ON":"OFF");
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
// Functions: Cat.M1 PDP context activate / deactivate
// ----------------------------------------------------------------

int8_t setContextActivate_WM01(void)    // Activate a PDP Context
{
    int8_t ret = RET_NOK;
    
    _parser->send("AT*RNDISDATA=1");  

    if(_parser->recv("OK")) 
    {
        devlog("Activate a PDP Context\r\n");
        
        ret = RET_OK;
    } 
    else 
    { 
        devlog("PDP Context Activation failed\r\n");        
    }

    return ret;
}

int8_t setContextDeactivate_WM01(void)  // Deactivate a PDP Context
{
    int8_t ret = RET_NOK;
    
    _parser->send("AT*RNDISDATA=0"); 

    if(_parser->recv("OK")) 
    {
        devlog("Deactivate a PDP Context\r\n");

        ret = RET_OK;
    } 
    else 
    { 
        devlog("PDP Context Deactivation failed\r\n");        
    }

    return ret;
}

// ----------------------------------------------------------------
// Functions: Cat.M1 Ping test
// ----------------------------------------------------------------

void printPingToHost_WM01(char *host, int pingnum)
{   
    Timer t;

    if((pingnum < 1) || (pingnum > 10)) 
    {
        devlog("The maximum number of sending Ping request range is 1-10, and the default value is 4\r\n");

        return;
    }

    _parser->set_timeout((1000 * pingnum) + 2000);

    if(_parser->send("AT*PING=%s,%d", host, pingnum) && _parser->recv("OK")) 
    {
        t.start();

        while(t.read_ms() < ((1000 * pingnum) + 2000))
        {
            pc.printf("%c", _parser->getc());
        }
    }
    
    _parser->set_timeout(WM01_DEFAULT_TIMEOUT);
    _parser->flush();
}