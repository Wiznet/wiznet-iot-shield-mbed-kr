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

// Functions: Module Status
void waitCatM1Ready(void);
int8_t setEchoStatus_BG96(bool onoff);
int8_t getUsimStatus_BG96(void);
int8_t getNetworkStatus_BG96(void);
int8_t checknSetApn_BG96(const char * apn);
int8_t getFirmwareVersion_BG96(char * version);
int8_t getImeiNumber_BG96(char * imei);

// Functions: DNS
int8_t getIpAddressByName_BG96(const char * name, char * ipstr);

// Functions: PDP context
int8_t setContextActivate_BG96(void);   // Activate a PDP Context
int8_t setContextDeactivate_BG96(void); // Deactivate a PDP Context
int8_t getIpAddress_BG96(char * ipstr);

// Functions: TCP/UDP Socket service
int8_t sockOpenConnect_BG96(const char * type, const char * addr, int port);
int8_t sockClose_BG96(void);
int8_t sendData_BG96(char * data, int len);
int8_t checkRecvData_BG96(void);
int8_t recvData_BG96(char * data, int * len);


Serial pc(USBTX, USBRX); // tx, rx

UARTSerial *_serial;
ATCmdParser *_parser;

DigitalOut _RESET_BG96(MBED_CONF_IOTSHIELD_CATM1_RESET);
DigitalOut _PWRKEY_BG96(MBED_CONF_IOTSHIELD_CATM1_PWRKEY);

// Destination (Remote Host)
// IP address and Port number
char dest_ip[] = "222.xxx.xxx.xxx";
int  dest_port = 50001;


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
    myprintf(">> Sample Code: TCP Client Send & Recv");
    myprintf("=================================================\r\n");
    
    setEchoStatus_BG96(OFF);
   
    getUsimStatus_BG96();
    
    getNetworkStatus_BG96();
    
    checknSetApn_BG96(CATM1_APN_SKT);
   
    setContextActivate_BG96();
        
#if 0    
    // DNS Sample
    const char dest_domain[] = "www.google.com";
    char ipstr[50];
    if(getIpAddressByName_BG96(dest_domain, ipstr)) {
        myprintf("DNS: %s [%s]\r\n", ipstr, dest_domain);
    } else {
        myprintf("DNS: failed\r\n");
    }
#endif    

    // TCP Client: Send and Receive
    myprintf("TCP Client Start - Connect to %s:%d\r\n", dest_ip, dest_port);
    
    if(sockOpenConnect_BG96("TCP", dest_ip, dest_port) == RET_OK) {
        myprintf("sockOpenConnect: success\r\n");
        
        char sendbuf[] = "Hello Cat.M1\r\n";         
        if(sendData_BG96(sendbuf, sizeof(sendbuf))) {
            myprintf("dataSend [%d] %s\r\n", sizeof(sendbuf), sendbuf);
        }
        
    } else {
        myprintf("sockOpen: failed\r\n");
        
        if(sockClose_BG96() == RET_OK) {
            myprintf("sockClose: success\r\n");
        }
    }    
    
    _parser->debug_on(DEBUG_DISABLE);
    
    while(1)
    {
        if(checkRecvData_BG96() == RET_OK) {
            // Data received
            char recvbuf[100] = {0, };
            int recvlen = 0;
            
            if(recvData_BG96(recvbuf, &recvlen) == RET_OK) {
                myprintf("dataRecv [%d] %s\r\n", recvlen, recvbuf);
                
                char * ptr = strstr(recvbuf, "exit");
                if(ptr != 0) break;
            }
        }
        
    }
    
    if(sockClose_BG96() == RET_OK) {
        myprintf("sockClose: success\r\n");
    }
    setContextDeactivate_BG96(); 
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

int8_t getIpAddressByName_BG96(const char * name, char * ipstr)
{
    char buf2[50];
    bool ok;
    int  err, ipcount, dnsttl;

    int8_t ret = RET_NOK;

    ok = ( _parser->send("AT+QIDNSGIP=1,\"%s\"", name)
            && _parser->recv("OK") 
            && _parser->recv("+QIURC: \"dnsgip\",%d,%d,%d", &err, &ipcount, &dnsttl) 
            && err==0 
            && ipcount > 0
        ); 

    if( ok ) {        
        _parser->recv("+QIURC: \"dnsgip\",\"%[^\"]\"", ipstr);       //use the first DNS value
        for( int i=0; i<ipcount-1; i++ )
            _parser->recv("+QIURC: \"dnsgip\",\"%[^\"]\"", buf2);   //and discrard the rest  if >1 
            
        ret = RET_OK;    
    }        
    return ret;           
}

// ----------------------------------------------------------------
// Functions: Cat.M1 PDP context activate / deactivate
// ----------------------------------------------------------------

int8_t setContextActivate_BG96(void) // Activate a PDP Context
{
    int8_t ret = RET_NOK;
    
    _parser->send("AT+QIACT=1");    
    if(_parser->recv("OK")) {
        devlog("Activate a PDP Context\r\n");
        ret = RET_OK;
    } else { 
        devlog("PDP Context Activation failed\r\n");        
    }    
    return ret;
}

int8_t setContextDeactivate_BG96(void) // Deactivate a PDP Context
{
    int8_t ret = RET_NOK;
    
    _parser->send("AT+QIDEACT=1");    
    if(_parser->recv("OK")) {
        devlog("Deactivate a PDP Context\r\n");
        ret = RET_OK;
    } else { 
        devlog("PDP Context Deactivation failed\r\n");        
    }
    return ret;
}

int8_t getIpAddress_BG96(char * ipstr) // IPv4 or IPv6
{
    int8_t ret = RET_NOK;    
    int id, state, type; // not used    

    _parser->send("AT+QIACT?");
    if(_parser->recv("+QIACT: %d,%d,%d,\"%[^\"]\"", &id, &state, &type, ipstr)
        && _parser->recv("OK")) {        
        ret = RET_OK;
    } 
    return ret;
}

// ----------------------------------------------------------------
// Functions: TCP/UDP socket service
// ----------------------------------------------------------------

int8_t sockOpenConnect_BG96(const char * type, const char * addr, int port)
{
    int8_t ret = RET_NOK;  
    int err = 1;
    int id = 0;
    
    bool done = false;
    Timer t;
    
    _parser->set_timeout(BG96_CONNECT_TIMEOUT);
    
    if((strcmp(type, "TCP") != 0) && (strcmp(type, "UDP") != 0)) {        
        return RET_NOK;
    }

    t.start();
    
    _parser->send("AT+QIOPEN=1,%d,\"%s\",\"%s\",%d", id, type, addr, port);
    do {        
        done = (_parser->recv("+QIOPEN: %d,%d", &id, &err) && (err == 0));        
    } while(!done && t.read_ms() < BG96_CONNECT_TIMEOUT);

    if(done) ret = RET_OK;

    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);    
    _parser->flush();
    
    return ret;
}

int8_t sockClose_BG96(void)
{
    int8_t ret = RET_NOK;
    int id = 0;
    
    _parser->set_timeout(BG96_CONNECT_TIMEOUT);
    
    if(_parser->send("AT+QICLOSE=%d", id) && _parser->recv("OK")) {
        ret = RET_OK;        
    }
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);
    
    return ret;
}

int8_t sendData_BG96(char * data, int len)
{
    int8_t ret = RET_NOK;
    int id = 0;
    bool done = false;
    
    _parser->set_timeout(BG96_SEND_TIMEOUT);
    
    _parser->send("AT+QISEND=%d,%ld", id, len);
    if( !done && _parser->recv(">") )
        done = (_parser->write(data, len) <= 0);

    if( !done )
        done = _parser->recv("SEND OK");    
    
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);
    
    return ret;
}

int8_t checkRecvData_BG96(void)
{
    int8_t ret = RET_NOK;
    int id = 0;
    char cmd[20];
    
    bool received = false;

    sprintf(cmd, "+QIURC: \"recv\",%d", id);
    _parser->set_timeout(1);
    received = _parser->recv(cmd);
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);
    
    if(received) ret = RET_OK;    
    return ret;
}

int8_t recvData_BG96(char * data, int * len)
{
    int8_t ret = RET_NOK;
    int id = 0;
    int recvCount = 0;
    
    _parser->set_timeout(BG96_RECV_TIMEOUT);   
     
    if( _parser->send("AT+QIRD=%d", id) && _parser->recv("+QIRD:%d\r\n",&recvCount) ) {
        if(recvCount > 0) {
            _parser->getc();
            _parser->read(data, recvCount);
            if(_parser->recv("OK")) {
                ret = RET_OK;
            } else {
                recvCount = 0;
            }
        }        
    }
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);    
    _parser->flush();
    
    *len = recvCount;
        
    return ret;
}
