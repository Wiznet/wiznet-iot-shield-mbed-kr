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

AnalogIn   tempVal(MBED_CONF_IOTSHIELD_SENSOR_TEMP);

float val = 0;          // 온도 측정 값 넣을 변수
float voltage = 0;    // 전압 계산 값 넣을 변수
float celsius = 0;    // 섭씨 계산 값 넣을 변수
float fahrenheit = 0; // 화씨 계산 값 넣을 변수

float getTemperature_F(float _celsius);
float getTemperature_C(float _voltage);

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
    myprintf(">> Sample Code: Temperature Sensor (LM35)");
    myprintf("=================================================\r\n");

    while(1) {
        // read the value from the sensor:
        val = tempVal.read_u16()/100;
        // 실제 전압 계산
        // 0~5V의 값을 구하기 위해 아래와 같이 5를 곱하고 1024를 나눈다.
        // (Analog값 * IC에 걸리는 전압 / Analog 10bit 분해능(1024))
        // IC에 걸리는 전압은 측정기로 측정

        voltage = val * 4.95 / 1024;
        celsius = getTemperature_C(voltage);
        fahrenheit = getTemperature_F(celsius);

        wait_ms(1000);
        myprintf("[ Temperature Data ]");
        myprintf("Sensor value : %.2f", val);
        myprintf("Volt : %.2f mV", voltage);

        myprintf("Celsius temp : %.2f C", celsius);
        myprintf("Fahrenheit temp : %.2f F", fahrenheit);
        wait_ms(1000);
    }
}

float getTemperature_F(float _celsius)
{
    // 섭씨에서 화씨 환산 -> (화씨온도 = 섭씽온*9/5+32)
    fahrenheit = celsius * 9.0 / 5.0 + 32.0;
    return fahrenheit;
}

float getTemperature_C(float _voltage)
{
    // LM35는 섭씨 1도당 10mV의 전위차를 갖는다.
    // 센서핀의 전압이 0.28V라면 280mV이므로 온도는 28도씨.
    // 100을 곱해서 섭씨 온도로 나타냄
    celsius = voltage * 100.0;
    return celsius;
}
