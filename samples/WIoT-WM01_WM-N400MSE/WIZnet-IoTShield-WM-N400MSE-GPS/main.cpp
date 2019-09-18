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

typedef struct
{
    int date;           // date yyyymmdd
    int utc;            // utc time hhmmss
    float lat;          // latitude. (-)dd.ddddd
    float lon;          // longitude. (-)dd.ddddd
    float spkm;         // speed over ground (km/h) xxx.x
    float cog;          // course over ground ddd.m
    char reli;          // reliability 'A', 'V'
    int lte;            // LTE status
    int emm;            // EMM reject
    int esm;            // ESM reject
    int ss;             // signal strength                              
    int nsat;           // number of satellites 0-12
    char satinfo1[8];   // satellites infomation ID-signal strength
    char satinfo2[8];   // satellites infomation ID-signal strength
    char satinfo3[8];   // satellites infomation ID-signal strength
    char satinfo4[8];   // satellites infomation ID-signal strength
} gps_data;

// Functions: Print information
void printInfo(void);

// Functions: Module Status
void waitCatM1Ready(void);
int8_t setEchoStatus_WM01(bool onoff);
int8_t getUsimStatus_WM01(void);
int8_t getNetworkStatus_WM01(void);

// Functions: GPS
int8_t setGpsOnOff_WM01(bool onoff);
int8_t getGpsInfo_WM01(gps_data *gps_info);

// Functions: String seperation
char *strsep(char **stringp, const char * delim);

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
    gps_data gps_info;

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

    if(setGpsOnOff_WM01(ON))
    {
        myprintf("GPS On : success\r\n");

        while(1)
        {
            if(getGpsInfo_WM01(&gps_info) == RET_OK)
            {
                myprintf("gps_info - reli     : %c", gps_info.reli);            // reliability 'A', 'V'           
                myprintf("gps_info - data     : %d", gps_info.date);            // date yyyymmdd    
                myprintf("gps_info - time     : %d", gps_info.utc);             // utc time hhmmss  
                myprintf("gps_info - lat      : %f", gps_info.lat / 100000);    // latitude. (-)dd.ddddd            
                myprintf("gps_info - lon      : %f", gps_info.lon / 100000);    // longitude. (-)dd.ddddd       
                myprintf("gps_info - spkm     : %.1f", gps_info.spkm);          // speed over ground (km/h) xxx.x   
                myprintf("gps_info - cog      : %.1f", gps_info.cog);           // course over ground ddd.m               
                myprintf("gps_info - lte      : %d", gps_info.lte);             // LTE status
                myprintf("gps_info - emm      : %d", gps_info.emm);             // EMM reject
                myprintf("gps_info - esm      : %d", gps_info.esm);             // ESM reject
                myprintf("gps_info - ss       : %d", gps_info.ss);              // signal strength                 
                myprintf("gps_info - nsat     : %d", gps_info.nsat);            // number of satellites 0-12
                myprintf("gps_info - satinfo1 : %s", gps_info.satinfo1);        // satellites infomation ID-signal strength
                myprintf("gps_info - satinfo2 : %s", gps_info.satinfo2);        // satellites infomation ID-signal strength
                myprintf("gps_info - satinfo3 : %s", gps_info.satinfo3);        // satellites infomation ID-signal strength
                myprintf("gps_info - satinfo4 : %s\r\n", gps_info.satinfo4);    // satellites infomation ID-signal strength
            }
        }
    }
    else
    {
        myprintf("GPS On : failed\r\n");
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
    myprintf(">> Sample Code: GPS Test");
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
        devlog("Turn Echo %s success\r\n", onoff ? "ON" : "OFF");

        ret = RET_OK;
    } 
    else 
    { 
        devlog("Turn Echo %s failed\r\n", onoff ? "ON" : "OFF");
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
// Functions: Cat.M1 GPS
// ----------------------------------------------------------------

int8_t setGpsOnOff_WM01(bool onoff)
{
    int8_t ret = RET_NOK;

    if(onoff == ON)
    {
        if(_parser->send("AT$$GPS") && _parser->recv("OK"))
        {
            devlog("GPS Rx start : success\r\n");

            ret = RET_OK;
        }
        else
        {
            devlog("GPS Rx start : failed\r\n");
        }    
    }
    else if(onoff == OFF)
    {
        if(_parser->send("AT$$GPSSTOP") && _parser->recv("OK"))
        {
            devlog("GPS Rx stop : success\r\n");

            ret = RET_OK;
        }
        else
        {
            devlog("GPS Rx stop : failed\r\n");
        }
    }

    return ret;
}

int8_t getGpsInfo_WM01(gps_data *gps_info)
{
    int8_t ret = RET_NOK;
    //char _buf[] = "20190910,111720,3737831,12711294,1,175,A,2,255,255,-31,7,1-32,255-255,11-100,17-22";
    char _buf[100] = {0, };
    char *_tmp[20] = {0, };
    char *gp;
    char *sp;
    int i = 0;

    // structure init : GPS info
    gps_info->date = gps_info->utc = gps_info->lte = gps_info->emm = gps_info->esm = gps_info->ss = gps_info->nsat = 0;
    gps_info->lat = gps_info->lon = gps_info->spkm = gps_info->cog = 0.0;
    gps_info->reli = 'V';
    memset(gps_info->satinfo1, 0x00, sizeof(gps_info->satinfo1));
    memset(gps_info->satinfo2, 0x00, sizeof(gps_info->satinfo2));
    memset(gps_info->satinfo3, 0x00, sizeof(gps_info->satinfo3));
    memset(gps_info->satinfo4, 0x00, sizeof(gps_info->satinfo4));

    _parser->flush();

    if(_parser->recv("$$GPS,%s\n", _buf))
    {
        sp = _buf;

        while((gp = strsep(&sp, ",")) != NULL)
        {
            _tmp[i] = gp;

            i++;
        }

        // convert data and store
        gps_info->date = atoi(_tmp[0]);
        gps_info->utc = atoi(_tmp[1]);
        gps_info->lat = atof(_tmp[2]);
        gps_info->lon = atof(_tmp[3]);
        gps_info->spkm = atof(_tmp[4]);
        gps_info->cog = atof(_tmp[5]);
        gps_info->reli = *_tmp[6];
        gps_info->lte = atoi(_tmp[7]);
        gps_info->emm = atoi(_tmp[8]);
        gps_info->esm = atoi(_tmp[9]);
        gps_info->ss = atoi(_tmp[10]);
        gps_info->nsat = atoi(_tmp[11]);
        memcpy(gps_info->satinfo1, (char *)_tmp[12], strlen(_tmp[12]));
        memcpy(gps_info->satinfo2, (char *)_tmp[13], strlen(_tmp[13]));
        memcpy(gps_info->satinfo3, (char *)_tmp[14], strlen(_tmp[14]));
        memcpy(gps_info->satinfo4, (char *)_tmp[15], strlen(_tmp[15]));

        ret = RET_OK;
    }

    return ret;
}

// ----------------------------------------------------------------
// Functions: String seperation
// ----------------------------------------------------------------

char *strsep(char **stringp, const char * delim)
{
    char *ptr = *stringp;

    if (ptr == NULL)
    {
        return NULL;
    }

    while (**stringp)
    {
        if (strchr(delim, **stringp) != NULL)
        {
            **stringp = 0x00;
            (*stringp)++;

            return ptr;
        }
        (*stringp)++;
    }
    *stringp = NULL;

    return ptr;
}