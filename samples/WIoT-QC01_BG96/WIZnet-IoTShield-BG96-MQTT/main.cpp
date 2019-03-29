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

/* MQTT */
#define MQTT_EOF                    0x1A

// Connection info: 
// HiveMQ Websocket client(http://www.hivemq.com/demos/websocket-client/)
#define MQTT_CLIENTID               "clientExample"
#define MQTT_USERID                 "testid"
#define MQTT_PASSWORD               "testpassword"
#define MQTT_QOS0                   0
#define MQTT_QOS1                   1
#define MQTT_QOS2                   2
#define MQTT_RETAIN                 0

#define MQTT_SAMPLE_TOPIC_A         "topic/openhouse-A"
#define MQTT_SAMPLE_TOPIC_B         "topic/openhouse-B"
#define MQTT_SAMPLE_TOPIC_C         "topic/openhouse-C"
#define MQTT_SAMPLE_TOPIC_D         "topic/openhouse-D"

/* MQTT Sample */
// MQTT connection state
enum {    
    MQTT_STATE_OPEN = 0,
    MQTT_STATE_CONNECT,
    MQTT_STATE_CONNECTED,
    MQTT_STATE_DISCON
};

char mqtt_broker_url[] = "broker.hivemq.com";
int mqtt_broker_port = 1883;
int mqtt_socket_id = 0;

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
    myprintf(">> Sample Code: MQTT Publish & Subscribe");
    myprintf("=================================================\r\n");

    setEchoStatus_BG96(OFF);
   
    getUsimStatus_BG96();
    
    getNetworkStatus_BG96();
    
    checknSetApn_BG96(CATM1_APN_SKT);
    
    int mqtt_state = MQTT_STATE_OPEN;
    
    // MQTT Connect phase
    //----------------------------------------
    myprintf("[MQTT] Try to connect to broker \"%s:%d\"\r\n", mqtt_broker_url, mqtt_broker_port);
    
    do {
        switch(mqtt_state) {
            case MQTT_STATE_OPEN:
                if(openMqttBroker_BG96(mqtt_broker_url, mqtt_broker_port) == RET_OK) {
                    myprintf("[MQTT] Socket open success\r\n");
                    mqtt_state = MQTT_STATE_CONNECT;
                } else {
                    myprintf("[MQTT] Socket open failed\r\n");
                }
            break;
                
            case MQTT_STATE_CONNECT:
                if(connectMqttBroker_BG96(MQTT_CLIENTID, MQTT_USERID, MQTT_PASSWORD) == RET_OK) {
                    myprintf("[MQTT] Connected, ClientID: \"%s\"\r\n", MQTT_CLIENTID);
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
    } while(mqtt_state != MQTT_STATE_CONNECTED);

    // MQTT Publish & Subscribe phase
    //----------------------------------------
    if(mqtt_state == MQTT_STATE_CONNECTED) {            
        char mqtt_message_b[] = "Hello IoT Open House B";
        char mqtt_message_c[] = "Hello IoT Open House C";
    
        // MQTT Subscribe
        if(setMqttSubscribeTopic_BG96(MQTT_SAMPLE_TOPIC_A, 1, MQTT_QOS0) == RET_OK) {
            myprintf("[MQTT] Subscribe Topic: \"%s\", ID: %d, QoS: %d\r\n", MQTT_SAMPLE_TOPIC_A, 1, MQTT_QOS0);
        }
#if 0        
        if(setMqttSubscribeTopic_BG96(MQTT_SAMPLE_TOPIC_D, 2, MQTT_QOS2) == RET_OK) {
            myprintf("[MQTT] Subscribe Topic: \"%s\", ID: %d, QoS: %d\r\n", MQTT_SAMPLE_TOPIC_D, 2, MQTT_QOS0);
        }
#endif        
        // MQTT Publish test
        if(sendMqttPublishMessage_BG96(MQTT_SAMPLE_TOPIC_B, MQTT_QOS1, MQTT_RETAIN, mqtt_message_b, strlen(mqtt_message_b)) == RET_OK) {
            myprintf("[MQTT] Published Topic: \"%s\", Message: \"%s\"\r\n", MQTT_SAMPLE_TOPIC_B, mqtt_message_b);
        }
        
        if(sendMqttPublishMessage_BG96(MQTT_SAMPLE_TOPIC_C, MQTT_QOS1, MQTT_RETAIN, mqtt_message_c, strlen(mqtt_message_c)) == RET_OK) {
            myprintf("[MQTT] Published Topic: \"%s\", Message: \"%s\"\r\n", MQTT_SAMPLE_TOPIC_C, mqtt_message_c);
        }
    }
    
    char buf_mqtt_topic[100] = {0, };
    char buf_mqtt_msg[200] = {0, };    
    int mqtt_msgid = 0;
    
    // MQTT Subscribe test    
    while(1)
    {
        // MQTT message received
        if(checkRecvMqttMessage_BG96(buf_mqtt_topic, &mqtt_msgid, buf_mqtt_msg) == RET_OK) {
            myprintf("[MQTT] Subscribed Topic: \"%s\" ID: %d, Message: \"%s\"\r\n", buf_mqtt_topic, mqtt_msgid, buf_mqtt_msg);
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
    static int msgid = 0;
    
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
    
    /*
    // Simple ver.    
    int8_t ret = RET_NOK;
    int id = 0;    
    bool received = false;
    
    _parser->set_timeout(1);
    received = _parser->recv("+QMTRECV: %d,%d,\"%[^\"]\",\"%[^\"]\"", &id, msgid, topic, msg);    
    _parser->set_timeout(BG96_DEFAULT_TIMEOUT);     
    
    if(received) ret = RET_OK;
    return ret;
    */    
}
