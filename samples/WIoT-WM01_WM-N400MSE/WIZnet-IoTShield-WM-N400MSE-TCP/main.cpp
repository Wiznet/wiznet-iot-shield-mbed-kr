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

// Destination (Remote Host)
// IP address and Port number
char dest_ip[] = "222.xxx.xxx.xxx";
int  dest_port = 8000;
int  protocol = 1;                  // 1 : TCP, 2 : UPD
int  packet_type = 0;               // 0 : ASCII, 1 : HEX, 2 : Binary

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

// Functions: TCP/UDP Socket service
int8_t sockOpenConnect_WM01(int protocol, const char *addr, int port, int pckttype);
int8_t sockClose_WM01(void);
int8_t sendData_WM01(char *data, int len);
int8_t recvData_WM01(char *data, int *len);

Serial pc(USBTX, USBRX); // tx, rx

UARTSerial *_serial;
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
    char sendbuf[] = "Hello Cat.M1\r\n";   

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

    wait_ms(3000);

    setContextActivate_WM01();

    // TCP Client: Send and Receive
    myprintf("TCP Client Start - Connect to %s:%d\r\n", dest_ip, dest_port);
    
    if(sockOpenConnect_WM01(protocol, dest_ip, dest_port, packet_type) == RET_OK)
    {
        myprintf("Socket Open Connect : success\r\n");

        if(sendData_WM01(sendbuf, strlen(sendbuf)))
        {
            myprintf("Data Send : success [%d] %s\r\n", strlen(sendbuf), sendbuf);
        }
        else
        {
            myprintf("Data Send : failed\r\n");
        } 
    } 
    else 
    {
        myprintf("Socket Open Connect : failed\r\n");
        
        if(sockClose_WM01() == RET_OK) 
        {
            myprintf("Socket Close : success\r\n");
        }
        else
        {
            myprintf("Socket Close : failed\r\n");
        }
    }    
    
    while(1)
    {
        // Data received
        char recvbuf[100] = {0, };
        int recvlen = 0;

        if(recvData_WM01(recvbuf, &recvlen) == RET_OK) 
        {
            myprintf("Data Receive [%d] %s\r\n", recvlen, recvbuf);
            
            char *ptr = strstr(recvbuf, "exit");

            if(ptr != 0)
            {
                break;
            }
        }
    }
    
    if(sockClose_WM01() == RET_OK) 
    {
        myprintf("Socket Close : success\r\n");
    }
    else
    {
        myprintf("Socket Close : failed\r\n");
    }

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
    myprintf(">> Sample Code: TCP Test");
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
// Functions: TCP/UDP socket service
// ----------------------------------------------------------------

int8_t sockOpenConnect_WM01(int protocol, const char *addr, int port, int pckttype)
{
    int8_t ret = RET_NOK;  
    bool done = false;
    int ok = 0;    // 0 : failure , 1 : success
    int id = 0;

    Timer t;
    
    _parser->set_timeout(WM01_CONNECT_TIMEOUT);
    
    if((protocol != 0) && (protocol != 1))  // 0 : TCP, 1 : UDP
    {        
        return RET_NOK;
    }

    t.start();
    
    _parser->send("AT+WSOCR=%d,%s,%d,%d,%d", id, addr, port, protocol, pckttype);
    
    do 
    {        
        done = (_parser->recv("+WSOCR:%d,%d", &ok,&id) && (ok == 1));
    } while(!done && t.read_ms() < WM01_CONNECT_TIMEOUT);

    if(done) 
    {
        if(_parser->send("AT+WSOCO=%d", id) && _parser->recv("+WSOCO:%d,%d,OPEN_WAIT", &ok, &id) && _parser->recv("OK"))
        {
            if(ok == 1)
            {
                if(_parser->recv("+WSOCO:%d,OPEN_CMPL", &id))
                {
                    ret = RET_OK;
                }
            }
        }
    }

    _parser->set_timeout(WM01_DEFAULT_TIMEOUT);    
    _parser->flush();
    
    return ret;
}

int8_t sockClose_WM01(void)
{
    int8_t ret = RET_NOK;
    int ok = 0;    // 0 : failure , 1 : success
    int id = 0;
    
    _parser->set_timeout(WM01_CONNECT_TIMEOUT);
    
    if(_parser->send("AT+WSOCL=%d", id) && _parser->recv("+WSOCL:%d,%d,CLOSE_WAIT", &ok, &id) &&_parser->recv("OK")) 
    {
        if(ok == 1)
        {
            ret = RET_OK;
        }        
    }

    _parser->set_timeout(WM01_DEFAULT_TIMEOUT);
    
    return ret;
}

int8_t sendData_WM01(char *data, int len)
{
    int8_t ret = RET_NOK;
    bool done = false;
    int ok = 0;    // 0 : failure , 1 : success
    int id = 0;

    _parser->set_timeout(WM01_SEND_TIMEOUT);
    _parser->send("AT+WSOWR=%d,%d,%s", id, len, data);

    if(_parser->recv("+WSOWR:%d,%d", &ok, &id) && _parser->recv("OK"))
    {
        if(ok == 1)    // success
        {
            ret = RET_OK;
        }
    }

    _parser->set_timeout(WM01_DEFAULT_TIMEOUT);
    
    return ret;
}

int8_t recvData_WM01(char *data, int *len)
{
    int8_t ret = RET_NOK;
    char _buf[100] = {0, };
    int id = 0;
    int i = 0;

    Timer t;
    
    _parser->set_timeout(WM01_RECV_TIMEOUT);

    t.start();

    while(t.read_ms() < WM01_RECV_TIMEOUT)
    {
        _buf[i] = _parser->getc();

        i++;
    }

    if(sscanf(_buf,"+WSORD:%d,%d,%[^\n]", &id, len, data))
    {
        ret = RET_OK;
    }

    _parser->set_timeout(WM01_DEFAULT_TIMEOUT);    
    _parser->flush();
        
    return ret;
}