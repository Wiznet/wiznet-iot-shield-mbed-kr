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

/* HTTP */
#define HTTP_STATUS_CODE_OK                         200

// Sample HTTP URL: Weather info by Korea Meteorological Administration
char request_url[] = "http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=4113552000";


// Functions: Module Status
void waitCatM1Ready(void);
int8_t setEchoStatus_BG96(bool onoff);
int8_t getUsimStatus_BG96(void);
int8_t getNetworkStatus_BG96(void);
int8_t checknSetApn_BG96(const char * apn);
int8_t getFirmwareVersion_BG96(char * version);

// Functions: HTTP send & recv
int8_t setHttpRequest_BG96(char * url, int len);
int8_t sendHttpRequest_BG96(int timeout, int * rsp_code, int * content_len);
int8_t getHttpResponse_BG96(int timeout, char * buf, int len);
void dumpHttpRespones_BG96(char * buf);

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
    myprintf(">> Sample Code: HTTP Send & Recv");
    myprintf("=================================================\r\n");
    
    setEchoStatus_BG96(OFF);
   
    getUsimStatus_BG96();
    
    getNetworkStatus_BG96();
    
    checknSetApn_BG96(CATM1_APN_SKT);
    
    // Set HTTP request URL
    if(setHttpRequest_BG96(request_url, strlen(request_url)-1) != RET_OK) {   
        myprintf("[HTTP] setHttpRequest failed\r\n");
        while(1){;}
    }
    
    int http_response_code = 0;
    int http_response_len = 0;
    
    if(sendHttpRequest_BG96(20, &http_response_code, &http_response_len)) {
        if(http_response_code == HTTP_STATUS_CODE_OK) {
            myprintf("[HTTP] 200 OK, Response content length: %d\r\n", http_response_len);
            
            char * http_buf;
            http_buf = (char*)calloc(http_response_len+2, sizeof(char)); // memory allocation
                        
            if(getHttpResponse_BG96(20, http_buf, http_response_len+2)) {
                dumpHttpRespones_BG96(http_buf);        
            }
            
            free(http_buf); // release
            
            myprintf("[HTTP] getHttpResponse success\r\n");
        } else {
            myprintf("[HTTP] sendHttpRequest failed - HTTP response code: %d\r\n", http_response_code);
        }
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
// Functions: Cat.M1 HTTP send & recv
// ----------------------------------------------------------------

int8_t setHttpRequest_BG96(char * req, int len)
{
    int8_t ret = RET_NOK;    
    bool done = false;
    
    _parser->set_timeout(BG96_CONNECT_TIMEOUT);
    
    _parser->send("AT+QHTTPURL=%d,%d", len, 5);    
    if( !done && _parser->recv("CONNECT\r\n") )
        done = (_parser->write(req, len) <= 0);
        
        if( !done ) {        
        done = (_parser->recv("OK"));        
        if(done) {
            devlog("Set HTTP request URL success: %s\r\n", req);
            ret = RET_OK;
        }
    }
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT); 
    
    return ret;
}


int8_t sendHttpRequest_BG96(int timeout, int * rsp_code, int * content_len)
{
    int8_t ret = RET_NOK;
    int err;
    
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT + (timeout * 1000));
    
    if( _parser->send("AT+QHTTPGET=%d", timeout) 
        && _parser->recv("+QHTTPGET: %d,%d,%d\r\n", &err, rsp_code, content_len)
        && (err == 0)) 
    {        
        devlog("The HTTP request is sent and the response is successful\r\n");        
        ret = RET_OK;            
    }    
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT); 
    
    return ret;
}


int8_t getHttpResponse_BG96(int timeout, char * buf, int len)
{
    int8_t ret = RET_NOK;
    bool done = false;
    
    if( _parser->send("AT+QHTTPREAD=%d", timeout) && _parser->recv("CONNECT\r\n")) {
        done = _parser->read(buf, len);
    }
    
    if(done) {
        if( _parser->recv("OK") && _parser->recv("+QHTTPREAD: 0")) {
            ret = RET_OK;
        }
    }            
    return ret;
}

void dumpHttpRespones_BG96(char * buf)
{
    myprintf("%s", buf);   
}
