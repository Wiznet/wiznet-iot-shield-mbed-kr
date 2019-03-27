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

#include "aws_iot_config.h"
#include "awscerts.h"

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
#define BG96_WAIT_TIMEOUT           3000
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

/* MQTT */
#define MQTT_EOF                                    0x1A
#define MQTT_QOS0                                   0
#define MQTT_QOS1                                   1
#define MQTT_QOS2                                   2
#define MQTT_RETAIN                                 0

/* SSL/TLS */
// Ciphersuites
#define BG96_TLS_RSA_WITH_AES_256_CBC_SHA           "0x0035"
#define BG96_TLS_RSA_WITH_AES_128_CBC_SHA           "0x002F"
#define BG96_TLS_RSA_WITH_RC4_128_SHA               "0x0005"
#define BG96_TLS_RSA_WITH_RC4_128_MD5               "0x0004"
#define BG96_TLS_RSA_WITH_3DES_EDE_CBC_SHA          "0x000A"
#define BG96_TLS_RSA_WITH_AES_256_CBC_SHA256        "0x003D"
#define BG96_TLS_ECDHE_RSA_WITH_RC4_128_SHA         "0xC011"
#define BG96_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA    "0xC012"
#define BG96_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA     "0xC013"
#define BG96_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA     "0xC014"
#define BG96_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256  "0xC027"
#define BG96_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384  "0xC028"
#define BG96_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256  "0xC02F"
#define BG96_TLS_SUPPORT_ALL                        "0xFFFF"

// SSL/TLS version
#define BG96_TLS_VERSION_SSL30                      0   // SSL3.0
#define BG96_TLS_VERSION_TLS10                      1   // TLS1.0
#define BG96_TLS_VERSION_TLS11                      2   // TLS1.1
#define BG96_TLS_VERSION_TLS12                      3   // TLS1.2
#define BG96_TLS_VERSION_ALL                        4

/* Debug message settings */
#define BG96_PARSER_DEBUG           DEBUG_DISABLE
#define CATM1_DEVICE_DEBUG          DEBUG_ENABLE 

/* MQTT Sample */
// MQTT connection state
enum {    
    MQTT_STATE_OPEN = 0,
    MQTT_STATE_CONNECT,
    MQTT_STATE_CONNECTED,
    MQTT_STATE_DISCON
};


// Functions: Module Status
void waitCatM1Ready(void);
int8_t setEchoStatus_BG96(bool onoff);
int8_t getUsimStatus_BG96(void);
int8_t getNetworkStatus_BG96(void);
int8_t checknSetApn_BG96(const char * apn);
int8_t getFirmwareVersion_BG96(char * version);
int8_t rebootCatm1_BG96(void);

// Functions: PDP context
int8_t setContextActivate_BG96(void);   // Activate a PDP Context
int8_t setContextDeactivate_BG96(void); // Deactivate a PDP Context
int8_t getIpAddress_BG96(char * ipstr);

// Functions: MQTT
int8_t openMqttBroker_BG96(char * url, int port);
int8_t connectMqttBroker_BG96(char * clientid, char * userid, char * password);
int8_t closeMqttBroker_BG96(void);
int8_t sendMqttPublishMessage_BG96(char * topic, int qos, int retain, char * msg, int len);
int8_t setMqttSubscribeTopic_BG96(char * topic, int msgid, int qos);
int8_t checkRecvMqttMessage_BG96(char * topic, int * msgid, char * msg);

// Functions: MQTT SSL/TLS enable
int8_t setMqttTlsEnable_BG96(bool enable);

// Functions: File system
int8_t saveFileToStorage_BG96(char * path, const char * buf, int len);
int8_t eraseFileStorageAll_BG96(void);
void dumpFileList_BG96(void);

// Functions: SSL/TLS config
int8_t setTlsCertificatePath_BG96(char * param, char * path);    // Root CA, Client certificate, Client privatekey
int8_t setTlsConfig_sslversion_BG96(int ver);                    // 0: SSL3.0 / 1: TLS1.0 / 2: TLS1.1 / 3: TLS1.2
int8_t setTlsConfig_ciphersuite_BG96(char * ciphersuite);        // Ciphersuite
int8_t setTlsConfig_seclevel_BG96(int seclevel);         
int8_t setTlsConfig_ignoreltime_BG96(bool enable);

/* BG96 Config for connect to AWS IoT */
#define AWS_IOT_BG96_SSLTLS_VERSION              4           // 4: All
#define AWS_IOT_BG96_SSLTLS_SECLEVEL             2           // 2: Manage server and client authentication if requested by the remote server
#define AWS_IOT_BG96_SSLTLS_IGNORELOCALTIME      1           // 1: Ignore validity check for certification
#define AWS_IOT_BG96_SSLTLS_CIPHERSUITE          BG96_TLS_RSA_WITH_AES_256_CBC_SHA

/* AWS IoT Sample functions */
int8_t aws_iot_connection_process(void);


Serial pc(USBTX, USBRX);    // USB debug

UARTSerial *_serial;        // Cat.M1 module    
ATCmdParser *_parser;

DigitalOut _RESET_BG96(MBED_CONF_IOTSHIELD_CATM1_RESET);
DigitalOut _PWRKEY_BG96(MBED_CONF_IOTSHIELD_CATM1_PWRKEY);

AnalogIn SENSOR_CDS(MBED_CONF_IOTSHIELD_SENSOR_CDS);    // light
AnalogIn SENSOR_LM35(MBED_CONF_IOTSHIELD_SENSOR_TEMP);  // temperature

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

bool flag_aws_publish = true;

void aws_flip() {
    flag_aws_publish = true;
}

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
    myprintf(">> Sample Code: AWS IoT Pub/Sub (Built-in TLS)");
    myprintf("=================================================\r\n");

    setEchoStatus_BG96(OFF);
   
    getUsimStatus_BG96();
    
    getNetworkStatus_BG96();
    
    checknSetApn_BG96(CATM1_APN_SKT);    
    
    myprintf("[FILE] Save and check AWS certificates\r\n")    

    /* Erase BG96 file storage */   
    if(eraseFileStorageAll_BG96() == RET_OK) {
        myprintf("[FILE] Erase BG96 storage complete\r\n");
    };

    /* Store AWS IoT certificate files to BG96 storage */    
    saveFileToStorage_BG96(AWS_IOT_ROOT_CA_FILENAME, aws_iot_rootCA, strlen(aws_iot_rootCA));
    saveFileToStorage_BG96(AWS_IOT_CERTIFICATE_FILENAME, aws_iot_certificate, strlen(aws_iot_certificate));
    saveFileToStorage_BG96(AWS_IOT_PRIVATE_KEY_FILENAME, aws_iot_private_key, strlen(aws_iot_private_key));
        
#if 0    
    dumpFileList_BG96(); // file list dump
#endif
        
    myprintf("[SSL/TLS] Set BG96 SSL/TLS configuration\r\n")

    /* BG96 SSL/TLS config */    
    // Set AWS IoT Certificate files
    setTlsCertificatePath_BG96("cacert", AWS_IOT_ROOT_CA_FILENAME);             // Root CA
    setTlsCertificatePath_BG96("clientcert", AWS_IOT_CERTIFICATE_FILENAME);     // Client certificate
    setTlsCertificatePath_BG96("clientkey", AWS_IOT_PRIVATE_KEY_FILENAME);      // Client privatekey

    // Set SSL/TLS config
    setTlsConfig_sslversion_BG96(AWS_IOT_BG96_SSLTLS_VERSION);    
    setTlsConfig_ciphersuite_BG96(AWS_IOT_BG96_SSLTLS_CIPHERSUITE);    
    setTlsConfig_seclevel_BG96(AWS_IOT_BG96_SSLTLS_SECLEVEL);    
    setTlsConfig_ignoreltime_BG96(AWS_IOT_BG96_SSLTLS_IGNORELOCALTIME);

    /* BG96 MQTT config: SSL/TLS enable */
    setMqttTlsEnable_BG96(true);    

    /* AWS IoT MQTT Client */
    char aws_iot_sub_topic[128] = {0, };
    char aws_iot_pub_topic[128] = {0, };    
    char buf_mqtt_topic[128] = {0, };
    char buf_mqtt_recv[AWS_IOT_MQTT_RX_BUF_LEN] = {0, };
    char buf_mqtt_send[AWS_IOT_MQTT_TX_BUF_LEN] = {0, };
    int mqtt_len = 0;
    int mqtt_msgid = 0;
    
    bool subscribe_complete = false;        
    
    Ticker flipper;    
    flipper.attach(&aws_flip, 60.0); // Publish messages every 1 minute
    
    // Temperary value for sample: Temperature value and color string
    int idx = 0;
    int temp = 25;
    char str_color[3][10] = {"Red", "Green", "Blue"};    
    
    // AWS IoT Topics
    sprintf(aws_iot_sub_topic, "$aws/things/%s/shadow/update/accepted", AWS_IOT_MY_THING_NAME);
    sprintf(aws_iot_pub_topic, "$aws/things/%s/shadow/update", AWS_IOT_MY_THING_NAME);
    
    myprintf("[MQTT] Connect to AWS IoT \"%s:%d\"\r\n", AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT);
    
    while(1)
    {
        if(aws_iot_connection_process() == MQTT_STATE_CONNECTED) {                        
            
            // MQTT Subscribe
            if(subscribe_complete != true) {                
                if(setMqttSubscribeTopic_BG96(aws_iot_sub_topic, 1, MQTT_QOS1) == RET_OK) {
                    myprintf("[MQTT] Topic subscribed: \"%s\"\r\n", aws_iot_sub_topic);
                    subscribe_complete = true;
                }
            }
            
            // MQTT Publish
            if(flag_aws_publish == true) {                
                
                mqtt_len = sprintf(buf_mqtt_send, "{\"state\":{\"reported\":{\"Temp\":\"%d\",\"Color\":\"%s\"}}}", temp+idx, str_color[idx]);          
                if(sendMqttPublishMessage_BG96(aws_iot_pub_topic, MQTT_QOS1, MQTT_RETAIN, buf_mqtt_send, mqtt_len) == RET_OK) {
                    myprintf("[MQTT] Message published: \"%s\", Message: %s\r\n", aws_iot_pub_topic, buf_mqtt_send);
                }
                
                if(idx >= 2) {idx = 0;} else {idx++;}  // Temperature value for example
                flag_aws_publish = false; // flag clear
            }
            
            // MQTT message received
            if(checkRecvMqttMessage_BG96(buf_mqtt_topic, &mqtt_msgid, buf_mqtt_recv) == RET_OK) {
                myprintf("[MQTT] Message arrived: Topic \"%s\" ID %d, Message %s\r\n", buf_mqtt_topic, mqtt_msgid, buf_mqtt_recv);                
            }
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

int8_t rebootCatm1_BG96(void)
{
    int8_t ret = RET_NOK;
    if(_parser->send("AT+CFUN=1,1") && _parser->recv("OK")) {
        devlog("Cat.M1 module reboot\r\n");        
        ret = RET_OK;
    }
    return ret;
}

// ----------------------------------------------------------------
// Functions: Cat.M1 MQTT Publish & Subscribe
// ----------------------------------------------------------------

int8_t openMqttBroker_BG96(char * url, int port)
{
    int8_t ret = RET_NOK;
    int id = 0;
    int result = 0;
    
    bool done = false;
    Timer t;
    
    t.start();    
    if(_parser->send("AT+QMTOPEN=%d,\"%s\",%d", id, url, port) && _parser->recv("OK")) {        
        do {        
            done = (_parser->recv("+QMTOPEN: %d,%d", &id, &result) && (result == 0));       
                 
            // MQTT Open: result code sample, refer to BG96_MQTT_Application_Note 
            if(result == 1) {
                devlog("AT+QMTOPEN result[%d]: %s", 1, "Wrong parameter");
            } else if (result == 2) {                
                devlog("AT+QMTOPEN result[%d]: %s", 2, "MQTT identifier is occupied");                
            } else if (result == 3) {
                devlog("AT+QMTOPEN result[%d]: %s", 3, "Failed to activate PDP");
            } else if (result == 4) {
                devlog("AT+QMTOPEN result[%d]: %s", 4, "Failed to parse domain name");
            } else if (result == 5) {
                devlog("AT+QMTOPEN result[%d]: %s", 5, "Network disconnection error");
            }                        
        } while(!done && t.read_ms() < BG96_CONNECT_TIMEOUT);
        
        if(done) {            
            ret = RET_OK;
        }
    }    
    _parser->flush();    
    
    return ret;
}

int8_t connectMqttBroker_BG96(char * clientid, char * userid, char * password)
{
    int8_t ret = RET_NOK;
    int id = 0;
    int result = 0;
    int ret_code = 0;
    
    bool done = false;
    Timer t;        
    
    if((userid != NULL) && (password != NULL)) {
        _parser->send("AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"", id, clientid, userid, password);
    } else {
        _parser->send("AT+QMTCONN=%d,\"%s\"", id, clientid);
    }
        
    t.start();
    if(_parser->recv("OK")) 
    {        
        do {        
            done = (_parser->recv("+QMTCONN: %d,%d,%d", &id, &result, &ret_code) 
                    && (result == 0) && (ret_code == 0));
            
            // MQTT Connect: result sample, refer to BG96_MQTT_Application_Note 
            if(result == 1) {
                devlog("AT+QMTCONN result[%d]: %s", 1, "Packet retransmission");
            } else if (result == 2) {
                devlog("AT+QMTCONN result[%d]: %s", 2, "Failed to send packet");
            }
            
            // MQTT Connect: ret_code sample, refer to BG96_MQTT_Application_Note 
            if(result == 1) {
                devlog("AT+QMTCONN ret_code[%d]: %s", 1, "Connection Refused: Unacceptable Protocol Version");
            } else if (result == 2) {
                devlog("AT+QMTCONN ret_code[%d]: %s", 2, "Connection Refused: Identifier Rejected");
            } else if (result == 3) {
                devlog("AT+QMTCONN ret_code[%d]: %s", 3, "Connection Refused: Server Unavailable");
            } else if (result == 4) {
                devlog("AT+QMTCONN ret_code[%d]: %s", 4, "Connection Refused: Bad User Name or Password");
            } else if (result == 5) {
                devlog("AT+QMTCONN ret_code[%d]: %s", 5, "Connection Refused: Not Authorized");
            }
        } while(!done && t.read_ms() < BG96_CONNECT_TIMEOUT*2);
        
        if(done) {            
            ret = RET_OK;
        }
    }        
    _parser->flush();
    
    return ret;        
}

int8_t closeMqttBroker_BG96(void)
{
    int8_t ret = RET_NOK;
    int id = 0;
    int result = 0;    
    
    bool done = false;
    Timer t;
    
    t.start();    
    if(_parser->send("AT+QMTDISC=%d", id) && _parser->recv("OK")) {
        do {        
            done = (_parser->recv("+QMTDISC: %d,%d", &id, &result));        
        } while(!done && t.read_ms() < BG96_CONNECT_TIMEOUT*2);
        
        if(done) {            
            ret = RET_OK;
        }
    }    
    _parser->flush();
    
    return ret;
}


int8_t sendMqttPublishMessage_BG96(char * topic, int qos, int retain, char * msg, int len)
{
    int8_t ret = RET_NOK;
    int id = 0;
    int result = 0;        
    int sent_msgid = 0;
    static int msgid;
    
    bool done = false;
    Timer t;
        
    if(qos != 0) {
        if(msgid < 0xffff) 
            msgid++;
        else 
            msgid = 0;
    }
            
    t.start();    
    _parser->send("AT+QMTPUB=%d,%d,%d,%d,\"%s\"", id, qos?msgid:0, qos, retain, topic);    
    
    if( !done && _parser->recv(">") )
        done = (_parser->write(msg, len) <= 0) & _parser->send("%c", MQTT_EOF);
    
    if(_parser->recv("OK")) {    
        do {        
            done = (_parser->recv("+QMTPUB: %d,%d,%d", &id, &sent_msgid, &result));        
        } while(!done && t.read_ms() < BG96_CONNECT_TIMEOUT*2);
        
        if(done) {
            ret = RET_OK;
        }
    }    
    _parser->flush();
    
    return ret;
}

int8_t setMqttSubscribeTopic_BG96(char * topic, int msgid, int qos)
{
    int8_t ret = RET_NOK;
    int id = 0;
    int result = 0;    
    
    int sent_msgid = 0;
    int qos_level = 0;
    
    bool done = false;
    Timer t;    
   
   _parser->set_timeout(BG96_CONNECT_TIMEOUT);
   
    t.start();    
    if(_parser->send("AT+QMTSUB=%d,%d,\"%s\",%d", id, msgid, topic, qos) && _parser->recv("OK")) {
        do {        
            done = (_parser->recv("+QMTSUB: %d,%d,%d,%d", &id, &sent_msgid, &result, &qos_level));        
        } while(!done && t.read_ms() < BG96_CONNECT_TIMEOUT);
        
        if(done) {
            ret = RET_OK;
        }
    }    
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);
    _parser->flush();
    
    return ret;
}

int8_t checkRecvMqttMessage_BG96(char * topic, int * msgid, char * msg)
{
    int8_t ret = RET_NOK;
    int id = 0;    
    int idx = 0;
    char * search_pt;
    bool done = false;
    bool received = false;    
    Timer t;
    
    _parser->set_timeout(1);
    received = _parser->recv("+QMTRECV: %d,%d,\"%[^\"]\",", &id, msgid, topic);    
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);
     
    if(received) {
        idx = 0;
        t.start();
        do {        
            _parser->read(&msg[idx++], 1);        
            search_pt = strstr(msg, "\r\n");
            if (search_pt != 0) {
                done = true; // break;
                ret = RET_OK;
            }
        } while(!done && (t.read_ms() < BG96_DEFAULT_TIMEOUT));                
        t.stop();
    }
    return ret;
}


// ----------------------------------------------------------------
// Functions: MQTT SSL/TLS enable
// ----------------------------------------------------------------

int8_t setMqttTlsEnable_BG96(bool enable) 
{
    int8_t ret = RET_NOK;
    
    int id = 0; // tcp connection id (0 - 6)
    int tls_ctxindex = 0; // ssl context index (0 - 5)
    
    if(_parser->send("AT+QMTCFG=\"SSL\",%d,%d,%d", id, enable?1:0, tls_ctxindex) && _parser->recv("OK")) {        
        ret = RET_OK;
    } else { 
        devlog("MQTT SSL/TLS enable failed\r\n");
    }    
    return ret;
}

// ----------------------------------------------------------------
// Functions: Cat.M1 File system
// ----------------------------------------------------------------

int8_t saveFileToStorage_BG96(char * path, const char * buf, int len)
{
    int8_t ret = RET_NOK;
    int timeout_sec = 30;
    
    bool done = false;
    int upload_size = 0;
    char checksum[10] = {0, };    
    
    _parser->set_timeout(BG96_WAIT_TIMEOUT);
        
    if(_parser->send("AT+QFUPL=\"%s\",%d,%d", path, len, timeout_sec) && _parser->recv("CONNECT")) {        
        done = _parser->write(buf, len);    
        if(done) {
            if(_parser->recv("+QFUPL: %d,%s\r\n", &upload_size, checksum) && _parser->recv("OK")) {
                if(len == upload_size) {
                    devlog("File saved: %s, %d, %s\r\n", path, upload_size, checksum);
                    ret = RET_OK;
                }
            }
        }
    }    
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);        
    if(ret != RET_OK) {
        devlog("Save a file to storage failed: %s\r\n", path);
    }
    _parser->flush();
    
    wait_ms(100);
    return ret;
}

int8_t eraseFileStorageAll_BG96(void)
{
    int8_t ret = RET_NOK;
    
    if(_parser->send("AT+QFDEL=\"*\"") && _parser->recv("OK")) {        
        ret = RET_OK;
    } else { 
        devlog("Erase storage failed\r\n");
    }
    
    wait_ms(100);
    return ret;
}

#define MAX_FILE_LIST           10
void dumpFileList_BG96(void)
{    
    char _buf[30] = {0, };
    int flen = {0, };
    int fcnt = 0;
    
    bool done = false;
    Timer t;
        
    t.start();
    if(_parser->send("AT+QFLST")) {        
        do {             
            if(_parser->recv("+QFLST: \"%[^\"]\",%d\r\n", _buf, &flen)) {                
                devlog("File[%d]: %s, %d\r\n", fcnt++, _buf, flen);
                memset(_buf, 0x00, sizeof(_buf));         
            }            
            else if(_parser->recv("OK")) {
                done = true;
            }
        } while(!done && t.read_ms() < BG96_WAIT_TIMEOUT);
    }
    t.stop();
} 


// ----------------------------------------------------------------
// Functions: SSL/TLS config
// ----------------------------------------------------------------

int8_t setTlsCertificatePath_BG96(char * param, char * path)
{
    int8_t ret = RET_NOK;    
    int tls_ctxindex = 0;               // ssl context index (0 - 5)
    
    if(_parser->send("AT+QSSLCFG=\"%s\",%d,\"%s\"", param, tls_ctxindex, path) && _parser->recv("OK")) {        
        ret = RET_OK;
    } else { 
        devlog("Set SSL/TLS certificate path failed: %s\r\n", param);
    }    
    return ret;
}

// 0: SSL3.0, 1: TLS1.0, 2: TLS1.1, 3: TLS1.2, 4: All
int8_t setTlsConfig_sslversion_BG96(int ver)
{
    int8_t ret = RET_NOK;    
    int tls_ctxindex = 0;                   // ssl context index (0 - 5)
    char param[] = "sslversion";            // ssl config paramter type       
    
    if(_parser->send("AT+QSSLCFG=\"%s\",%d,%d", param, tls_ctxindex, ver) && _parser->recv("OK")) {        
        ret = RET_OK;
    } else { 
        devlog("Set SSL/TLS version failed: %d\r\n", ver);
    }    
    return ret;
}

int8_t setTlsConfig_ciphersuite_BG96(char * ciphersuite)    // refer to SSL manual
{
    int8_t ret = RET_NOK;
    int tls_ctxindex = 0;                   // ssl context index (0 - 5)
    char param[] = "ciphersuite";           // ssl config paramter type       
    
    if(_parser->send("AT+QSSLCFG=\"%s\",%d,%s", param, tls_ctxindex, ciphersuite) && _parser->recv("OK")) {        
        ret = RET_OK;
    } else { 
        devlog("Set SSL/TLS Ciphersuite failed: %d\r\n", ciphersuite);
    }
    return ret;
}

int8_t setTlsConfig_seclevel_BG96(int seclevel)
{
    int8_t ret = RET_NOK;    
    int tls_ctxindex = 0;                   // ssl context index (0 - 5)
    char sslconfig[] = "seclevel";          // ssl config paramter type
    
    if(_parser->send("AT+QSSLCFG=\"%s\",%d,%d", sslconfig, tls_ctxindex, seclevel) && _parser->recv("OK")) {        
        ret = RET_OK;
    } else { 
        devlog("Set SSL/TLS authentication mode failed: %d\r\n", seclevel);
    }
    return ret;
}
  
int8_t setTlsConfig_ignoreltime_BG96(bool enable)
{
    int8_t ret = RET_NOK;    
    int tls_ctxindex = 0;                   // ssl context index (0 - 5)
    char sslconfig[] = "ignorelocaltime";   // ssl config paramter type
    
    if(_parser->send("AT+QSSLCFG=\"%s\",%d,%d", sslconfig, tls_ctxindex, enable?1:0) && _parser->recv("OK")) {        
        ret = RET_OK;
    } else { 
        devlog("Set SSL/TLS ignore validity check option failed: %s\r\n", sslconfig);
    }
    return ret;
}


// ----------------------------------------------------------------
// Functions: AWS IoT samples
// ----------------------------------------------------------------

int8_t aws_iot_connection_process(void)
{   
    static int8_t mqtt_state;

    switch(mqtt_state) {
        case MQTT_STATE_CONNECTED:        
            break;
            
        case MQTT_STATE_OPEN:
            if(openMqttBroker_BG96(AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT) == RET_OK) {
                myprintf("[MQTT] Socket open success\r\n");
                mqtt_state = MQTT_STATE_CONNECT;
            } else {
                myprintf("[MQTT] Socket open failed\r\n");
            }
            break;
            
        case MQTT_STATE_CONNECT:
            if(connectMqttBroker_BG96(AWS_IOT_MQTT_CLIENT_ID, NULL, NULL) == RET_OK) {
                myprintf("[MQTT] Connected, ClientID: \"%s\"\r\n", AWS_IOT_MQTT_CLIENT_ID);
                mqtt_state = MQTT_STATE_CONNECTED;
            } else {
                myprintf("[MQTT] Connect failed\r\n");
                mqtt_state = MQTT_STATE_DISCON;
            }
            break;
        
        case MQTT_STATE_DISCON:
            if(closeMqttBroker_BG96() == RET_OK) {
                myprintf("[MQTT] Disconnected\r\n");                    
            }
            mqtt_state = MQTT_STATE_OPEN;
            break;
            
        default:                
            mqtt_state = MQTT_STATE_OPEN;
            break;
    }
    
    return mqtt_state;
}