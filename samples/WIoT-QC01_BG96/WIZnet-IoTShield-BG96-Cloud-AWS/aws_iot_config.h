/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file aws_iot_config.h
 * @brief AWS IoT specific configuration file
 */

#ifndef SRC_SHADOW_IOT_SHADOW_CONFIG_H_
#define SRC_SHADOW_IOT_SHADOW_CONFIG_H_

#define AWS_IOT_MQTT_HOST              (char*)"a3uz5t2xxxxxxx-ats.iot.ap-northeast-2.amazonaws.com"
#define AWS_IOT_MQTT_PORT              8883
#define AWS_IOT_MQTT_CLIENT_ID         (char*)"Your_Thing_Name" 
#define AWS_IOT_MY_THING_NAME          (char*)"Your_Thing_Name"
#define AWS_IOT_ROOT_CA_FILENAME       (char*)"AmazonRootCA1.pem"           
#define AWS_IOT_CERTIFICATE_FILENAME   (char*)"certificate.pem.crt"        
#define AWS_IOT_PRIVATE_KEY_FILENAME   (char*)"private.pem.key"     

// MQTT PubSub
#define AWS_IOT_MQTT_TX_BUF_LEN 1024                      
#define AWS_IOT_MQTT_RX_BUF_LEN 1024                      
#define AWS_IOT_MQTT_NUM_SUBSCRIBE_HANDLERS 5

#endif /* SRC_SHADOW_IOT_SHADOW_CONFIG_H_ */

