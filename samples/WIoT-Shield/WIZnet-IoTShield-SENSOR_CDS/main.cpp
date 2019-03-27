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

#define myprintf(f_, ...)           {pc.printf("\r\n[MAIN] ");  pc.printf((f_), ##__VA_ARGS__);}

// Sensors
#define MBED_CONF_IOTSHIELD_SENSOR_CDS              A0
#define MBED_CONF_IOTSHIELD_SENSOR_TEMP             A1
#define LEDPIN                                      D13

Serial pc(USBTX, USBRX);    // USB debug

DigitalOut  LED(LEDPIN);
AnalogIn    cdsVal(MBED_CONF_IOTSHIELD_SENSOR_CDS);

float val = 0;          // CDS 측정 값 넣을 변수

void serialPcInit(void)
{
    pc.baud(115200);
    pc.format(8, Serial::None, 1);
}

// ----------------------------------------------------------------
// Main routine
// ----------------------------------------------------------------

int main()
{
    serialPcInit();

    myprintf("System Init Complete\r\n");

    myprintf("WIZnet IoT Shield for Arm MBED");
    myprintf("LTE Cat.M1 Version");
    myprintf("=================================================");
    myprintf(">> Sample Code: CDS sensor");
    myprintf("=================================================\r\n");

    while(1) {
        val = cdsVal.read_u16()/100;
        //myprintf("Sensor value : %.2f", val);
        wait_ms(50);

        if(val > 512) { // 조도 저항 값이 낮기 때문에 아날로그 값이 높음
            LED = 0;
        } else { // 손가락으로 가리면 조도 저항값이 높아지기 때문에 아날로그 값이 낮아진다
            LED = 1;
        }
    }
}


