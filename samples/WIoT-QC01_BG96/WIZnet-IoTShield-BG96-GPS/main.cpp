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


#include <string>
#include "mbed.h"

#define RET_OK                      1
#define RET_NOK                     -1
#define DEBUG_ENABLE                1
#define DEBUG_DISABLE               0
#define ON                          1
#define OFF                         0

#define MAX_BUF_SIZE                1024

#define BG96_APN_PROTOCOL_IPv4      1
#define BG96_APN_PROTOCOL_IPv6      2
#define BG96_DEFAULT_TIMEOUT        1000
#define BG96_CONNECT_TIMEOUT        15000
#define BG96_SEND_TIMEOUT           500
#define BG96_RECV_TIMEOUT           500

#define BG96_APN_PROTOCOL           BG96_APN_PROTOCOL_IPv6
#define BG96_DEFAULT_BAUD_RATE      115200
#define BG96_PARSER_DELIMITER       "\r\n"

#define CATM1_APN_SKT               "lte-internet.sktelecom.com"

#define CATM1_DEVICE_NAME_BG96      "BG96"
#define DEVNAME                     CATM1_DEVICE_NAME_BG96

#define devlog(f_, ...)             if(CATM1_DEVICE_DEBUG == DEBUG_ENABLE) { pc.printf("\r\n[%s] ", DEVNAME);  pc.printf((f_), ##__VA_ARGS__); }
#define myprintf(f_, ...)           {pc.printf("\r\n[MAIN] ");  pc.printf((f_), ##__VA_ARGS__);}

/* Pin configuraiton */
// Cat.M1
#define MBED_CONF_IOTSHIELD_CATM1_TX                D8
#define MBED_CONF_IOTSHIELD_CATM1_RX                D2
#define MBED_CONF_IOTSHIELD_CATM1_RESET             D7
#define MBED_CONF_IOTSHIELD_CATM1_PWRKEY            D9

// Sensors
#define MBED_CONF_IOTSHIELD_SENSOR_CDS              A0
#define MBED_CONF_IOTSHIELD_SENSOR_TEMP             A1

/* Debug message settings */
#define BG96_PARSER_DEBUG           DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG          DEBUG_ENABLE 


typedef struct gps_data_t {
    float utc;      // hhmmss.sss
    float lat;      // latitude. (-)dd.ddddd
    float lon;      // longitude. (-)dd.ddddd
    float hdop;     // Horizontal precision: 0.5-99.9
    float altitude; // altitude of antenna from sea level (meters) 
    int fix;        // GNSS position mode 2=2D, 3=3D
    float cog;      // Course Over Ground ddd.mm
    float spkm;     // Speed over ground (Km/h) xxxx.x
    float spkn;     // Speed over ground (knots) xxxx.x
    char date[7];   // data: ddmmyy
    int nsat;       // number of satellites 0-12
} gps_data;


// Functions: Module Status
void waitCatM1Ready(void);
int8_t setEchoStatus_BG96(bool onoff);
int8_t getUsimStatus_BG96(void);
int8_t getNetworkStatus_BG96(void);
int8_t checknSetApn_BG96(const char * apn);
int8_t getFirmwareVersion_BG96(char * version);
int8_t getImeiNumber_BG96(char * imei);

// Functions: GPS
int8_t setGpsOnOff_BG96(bool onoff);
int8_t getGpsLocation_BG96(gps_data *data);


Serial pc(USBTX, USBRX);    // USB debug

UARTSerial *_serial;        // Cat.M1 module    
ATCmdParser *_parser;

DigitalOut _RESET_BG96(MBED_CONF_IOTSHIELD_CATM1_RESET);
DigitalOut _PWRKEY_BG96(MBED_CONF_IOTSHIELD_CATM1_PWRKEY);

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
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);
}

void catm1DeviceInit(void)
{
    serialDeviceInit(   MBED_CONF_IOTSHIELD_CATM1_TX, 
                        MBED_CONF_IOTSHIELD_CATM1_RX, 
                        BG96_DEFAULT_BAUD_RATE);
                        
    serialAtParserInit( BG96_PARSER_DELIMITER, 
                        BG96_PARSER_DEBUG);
}

void catm1DeviceReset_BG96(void)
{
    _RESET_BG96 = 1;
    _PWRKEY_BG96 = 1;
    wait_ms(300);
    
    _RESET_BG96 = 0;
    _PWRKEY_BG96 = 0;
    wait_ms(400);
    
    _RESET_BG96 = 1;    
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
    
    catm1DeviceReset_BG96();
    
    waitCatM1Ready();
    
    wait_ms(5000);
            
    myprintf("System Init Complete\r\n");
        
    myprintf("WIZnet IoT Shield for Arm MBED");
    myprintf("LTE Cat.M1 Version");
    myprintf("=================================================");
    myprintf(">> Target Board: WIoT-QC01 (Quectel BG96)");
    myprintf(">> Sample Code: GPS");
    myprintf("=================================================\r\n");
    
    setEchoStatus_BG96(OFF);
   
    getUsimStatus_BG96();
    
    getNetworkStatus_BG96();
    
    checknSetApn_BG96(CATM1_APN_SKT);
    
    // GPS information structure
    gps_data gps_info;    
    
    if(setGpsOnOff_BG96(ON) == RET_OK) {        
        myprintf("GPS On\r\n")

        while(1) {
            if(getGpsLocation_BG96(&gps_info) == RET_OK) {
                myprintf("Get GPS information >>>");
                myprintf("gps_info - utc: %6.3f", gps_info.utc)             // utc: hhmmss.sss
                myprintf("gps_info - lat: %2.5f", gps_info.lat)             // latitude: (-)dd.ddddd
                myprintf("gps_info - lon: %2.5f", gps_info.lon)             // longitude: (-)dd.ddddd
                myprintf("gps_info - hdop: %2.1f", gps_info.hdop)           // Horizontal precision: 0.5-99.9
                myprintf("gps_info - altitude: %2.1f", gps_info.altitude)   // altitude of antenna from sea level (meters)
                myprintf("gps_info - fix: %d", gps_info.fix)                // GNSS position mode: 2=2D, 3=3D
                myprintf("gps_info - cog: %3.2f", gps_info.cog)             // Course Over Ground: ddd.mm
                myprintf("gps_info - spkm: %4.1f", gps_info.spkm)           // Speed over ground (Km/h): xxxx.x
                myprintf("gps_info - spkn: %4.1f", gps_info.spkn)           // Speed over ground (knots): xxxx.x            
                myprintf("gps_info - date: %s", gps_info.date)              // data: ddmmyy
                myprintf("gps_info - nsat: %d\r\n", gps_info.nsat)          // number of satellites: 0-12
            } else {
                myprintf("Failed to get GPS information\r\n");
            }
            wait_ms(1000);
        }       
#if 0        
        if(setGpsOnOff_BG96(OFF) == RET_OK) {
            myprintf("GPS Off\r\n")
        }
#endif
        
    } else {
        myprintf("GPS On failed\r\n")
    }
}

// ----------------------------------------------------------------
// Functions: Cat.M1 Status
// ----------------------------------------------------------------

void waitCatM1Ready(void)
{
    while(1) 
    {   
        if(_parser->recv("RDY")) 
        {
            myprintf("BG96 ready\r\n");
            return ;
        }
        else if(_parser->send("AT") && _parser->recv("OK"))
        {
            myprintf("BG96 already available\r\n");
            return ;
        }        
    }        
}

int8_t setEchoStatus_BG96(bool onoff)
{
    int8_t ret = RET_NOK;
    char _buf[10];        
    
    sprintf((char *)_buf, "ATE%d", onoff);    
    
    if(_parser->send(_buf) && _parser->recv("OK")) {        
        devlog("Turn Echo %s success\r\n", onoff?"ON":"OFF");
        ret = RET_OK;
    } else { 
        devlog("Turn Echo %s failed\r\n", onoff?"ON":"OFF");
    }    
    return ret;
}
 
int8_t getUsimStatus_BG96(void)
{
    int8_t ret = RET_NOK;
    
    _parser->send("AT+CPIN?");    
    if(_parser->recv("+CPIN: READY") && _parser->recv("OK")) {
        devlog("USIM Status: READY\r\n");
        ret = RET_OK;
    } else { 
        devlog("Retrieving USIM Status failed\r\n");        
    }
    return ret;
}

int8_t getNetworkStatus_BG96(void)
{
    int8_t ret = RET_NOK;    
    
    if(_parser->send("AT+QCDS") && _parser->recv("+QCDS: \"SRV\"") && _parser->recv("OK")) {
        devlog("Network Status: attached\r\n");
        ret = RET_OK;
    } else if (_parser->send("AT+QCDS") && _parser->recv("+QCDS: \"LIMITED\"") && _parser->recv("OK")) {
        devlog("Network Status: limited\r\n");
        ret = RET_OK;    
    } else { 
        devlog("Network Status: Error\r\n");        
    }
    return ret;
}

int8_t checknSetApn_BG96(const char * apn) // Configure Parameters of a TCP/IP Context
{       
    char resp_str[100];
    
    uint16_t i = 0;
    char * search_pt;
    
    memset(resp_str, 0, sizeof(resp_str));
    
    devlog("Checking APN...\r\n");
    
    _parser->send("AT+QICSGP=1");
    
    while(1)
    {
        _parser->read(&resp_str[i++], 1);        
        search_pt = strstr(resp_str, "OK\r\n");
        if (search_pt != 0)
        {
            break;
        }
    }
    
    search_pt = strstr(resp_str, apn);
    if (search_pt == 0)
    {
        devlog("Mismatched APN: %s\r\n", resp_str);
        devlog("Storing APN %s...\r\n", apn);
        if(!(_parser->send("AT+QICSGP=1,%d,\"%s\",\"\",\"\",0", BG96_APN_PROTOCOL, apn) && _parser->recv("OK")))
        {
            return RET_NOK; // failed
        }
    }    
    devlog("APN Check Done\r\n");    
    
    return RET_OK;
}

int8_t getFirmwareVersion_BG96(char * version)
{
    int8_t ret = RET_NOK;
    
    if(_parser->send("AT+QGMR") && _parser->recv("%s\n", version) && _parser->recv("OK"))
    {           
        ret = RET_OK;
    } 
    return ret;
}

int8_t getImeiNumber_BG96(char * imei)
{
    int8_t ret = RET_NOK;    
    
    if(_parser->send("AT+CGSN") && _parser->recv("%s\n", imei) && _parser->recv("OK"))
    { 
        ret = RET_OK;
    } 
    return ret;
}

// ----------------------------------------------------------------
// Functions: Cat.M1 GPS
// ----------------------------------------------------------------

int8_t setGpsOnOff_BG96(bool onoff)
{
    int8_t ret = RET_NOK;
    char _buf[15];        
    
    sprintf((char *)_buf, "%s", onoff ? "AT+QGPS=2" : "AT+QGPSEND");
    
    if(_parser->send(_buf) && _parser->recv("OK")) {
        devlog("GPS Power: %s\r\n", onoff ? "On" : "Off");
        ret = RET_OK;
    } else { 
        devlog("Set GPS Power %s failed\r\n", onoff ? "On" : "Off");        
    }
    return ret;    
}


int8_t getGpsLocation_BG96(gps_data *data)
{
    int8_t ret = RET_NOK;    
    char _buf[100];
    
    bool ok = false;    
    Timer t;
    
    // Structure init: GPS info
    data->utc = data->lat = data->lon = data->hdop= data->altitude = data->cog = data->spkm = data->spkn = data->nsat=0.0;
    data->fix=0;
    memset(&data->date, 0x00, 7);
    
    // timer start
    t.start();
    
    while( !ok && (t.read_ms() < BG96_CONNECT_TIMEOUT ) ) {
        _parser->flush();    
        _parser->send((char*)"AT+QGPSLOC=2"); // MS-based mode        
        ok = _parser->recv("+QGPSLOC: ");
        if(ok) {
            _parser->recv("%s\r\n", _buf);
            sscanf(_buf,"%f,%f,%f,%f,%f,%d,%f,%f,%f,%6s,%d",
                          &data->utc, &data->lat, &data->lon, &data->hdop,
                          &data->altitude, &data->fix, &data->cog,
                          &data->spkm, &data->spkn, data->date, &data->nsat);            
            ok = _parser->recv("OK");
        }         
    }
    
    if(ok == true) ret = RET_OK;
    
    return ret;
}
