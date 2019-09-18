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

char periodic_tau[] = "10100101";
char active_time[] = "00100100";

// Functions: Print information
void printInfo(void);

// Functions: Module Status
void waitCatM1Ready(void);
int8_t setEchoStatus_WM01(bool onoff);
int8_t getUsimStatus_WM01(void);
int8_t getNetworkStatus_WM01(void);

// Functions: PSM (Power Saving Mode)
int8_t setPsmActivate_WM01(char *Requested_Periodic_TAU, char *Requested_Active_Time);
int8_t setPsmDeactivate_WM01(void);

// Functions: Network time
int8_t getNetworkTimeLocal_WM01(char *time);
void setFlagGettime(void);
void clearFlagGettime(void);

Serial pc(USBTX, USBRX);    // USB debug

UARTSerial *_serial;        // Cat.M1 module    
ATCmdParser *_parser;

DigitalOut _RESET_WM01(MBED_CONF_IOTSHIELD_CATM1_RESET);
DigitalOut _PWRKEY_WM01(MBED_CONF_IOTSHIELD_CATM1_PWRKEY);

Ticker flip;

bool flag_gettime = false;

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
    bool psm_en = false;
    char time[30] = {0, };
    float elapsed_time_sec = 0; 

    Timer t;

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


#if 0
// PSM disable   
    setPsmDeactivate_WM01();
#endif    

    if(psm_en != true) 
    {
        if(setPsmActivate_WM01(periodic_tau, active_time) == RET_OK)
        {
            myprintf("Cat.M1 PSM enable, Device reboot\r\n");    

            // Cat.M1 reboot
            catm1DeviceReset_WM01();

            waitCatM1Ready();
        } 
        else 
        {
            myprintf("Cat.M1 PSM enable failed\r\n");
        }            
    }

    // Timer event callback       
    flip.attach(callback(&setFlagGettime), 1.0);

    while(1)
    {
        if(flag_gettime) 
        {
            if(getNetworkTimeLocal_WM01(time) == RET_OK) 
            {
                if(elapsed_time_sec > 0) 
                {
                    t.stop();

                    myprintf("Cat.M1 Active, Sleep time: %.2fsec", elapsed_time_sec);                    

                    elapsed_time_sec = 0;   
                }    

                myprintf("%s", time);
            } 
            else 
            {
                if(elapsed_time_sec == 0) 
                {            
                    t.reset();        
                    t.start();         

                    myprintf("%s", "PSM Power Down Start");               
                }

                elapsed_time_sec = t.read();

                myprintf("Cat.M1 PSM, %.2f", elapsed_time_sec);               
            }

            clearFlagGettime();
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
    myprintf(">> Sample Code: PSM Test");
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
// Functions: Cat.M1 PSM activate / deactivate
// ----------------------------------------------------------------

int8_t setPsmActivate_WM01(char *Requested_Periodic_TAU, char *Requested_Active_Time)
{
    int8_t ret = RET_NOK;

    if(_parser->send("AT+CPSMS=1,,,\"%s\",\"%s\"", Requested_Periodic_TAU, Requested_Active_Time) && _parser->recv("OK")) 
    {
        devlog("PSM activate : success\r\n");

        ret = RET_OK;
    }

    return ret;
}

int8_t setPsmDeactivate_WM01(void)
{
    int8_t ret = RET_NOK;
    
    if(_parser->send("AT+CPSMS=0") && _parser->recv("OK"))
    {
        devlog("PSM deactivate : success\r\n");
    }
    
    return ret;
}

// ----------------------------------------------------------------
// Functions: Cat.M1 Network time
// ----------------------------------------------------------------

int8_t getNetworkTimeLocal_WM01(char *time)
{
    int8_t ret = RET_NOK;
    
    if(_parser->send("AT$$MSTIME?") && _parser->recv("$$MSTIME:%*[^,],%s\n", time) && _parser->recv("OK"))  
    {
        //devlog("Get current local time success\r\n");      

        ret = RET_OK;
    }

    return ret;
}

void setFlagGettime(void)
{
    flag_gettime = true;
}

void clearFlagGettime(void)
{
    flag_gettime = false;
}