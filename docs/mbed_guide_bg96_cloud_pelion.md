# MBED 기반의 Cat.M1 Pelion IoT Platfrom 연동 가이드

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [예제 코드 빌드 및 실행](#Step-4-Build-and-Run)

<a name="Prerequisites"></a>
## 시작하기 전에

> * 하드웨어 설정과 개발환경 구축은 **[MBED 기반으로 Cat.M1 디바이스 개발 시작하기][mbed-getting-started]** 문서에 상세히 설명되어 있습니다.
>
> * Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]** 를 기준으로 작성되었습니다.
>
> * 본 예제는 아래 링크의 예제를 참고하여 제작하였으며, HW 구성의 경우 외장형 모뎀을 모듈로 변경하였고 SW의 경우 버튼 카운트에 온도를 센싱하여 주기적으로 Pelion IoT Platform으로 전송하도록 수정하였습니다.
>   * [BG96_K6xF_pelion-example-frdm][link-mbed-bg96-pelion-example]

### Development Environment
* **[Arm MBED Web Compiler][link-mbed-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [FRDM-K64F][link-frdm-k64f] | WIoT-QC01 (BG96) |

<a name="Step-1-Overview"></a>
## 소개
Mbed Pelion IoT Platform은 무엇입니까?

Mmbed Pelion IoT Platform은 Mmbed OS 디바이스와 연동이 가능하며 디바이스 관리, 연결 관리, 데이터 관리 등과 같은 기능을 제공하는 Platform 입니다. 이를 통해 인터넷 연결 제품(센서, 액추에이터, 내장형 마이크로 컨트롤러, 스마트 애플리케이션 등)과 Mbed Pelion IoT Platform 간에 안전한 양방향 통신을 할 수 있으며 여러 디바이스의 원격 데이터를 수집 및 모니터링 하고 해당 데이터를 저장하여 분석할 수 있습니다. 또한 Mbed Ready Boards를 활용하며 보다 간편하게 Mbed Pelion IoT Platform와 연동하여 사용할 수 있습니다.
 
출처 : [Using Mbed OS devices and the Pelion IoT Platform](https://os.mbed.com/docs/mbed-os/v5.13/mbed-os-pelion/index.html)

![][1]

본 문서에서는 Cat M1 모듈을 사용하여, Mbed Pelion Platform 서비스에 접속하는 방법에 대한 가이드를 제공합니다.
FRDM-K64F 외의 다른 디바이스 및 다른 무선 기술을 사용하신다면 다음 링크를 참고하시기 바랍니다.

> [Pelion Device Management IoT Connection Tutorial](https://os.mbed.com/guides/connect-device-to-pelion/)


이 가이드 문서는 아래와 같은 과정으로 구성되어 있습니다.

1. Import Device & project
2. Modify parameters
3. Compile
4. Demo


<a name="Step-4-Build-and-Run"></a>

## 예제 코드 빌드 및 실행

### 1. Import Device & project

아래의 FREM-K64F Board 링크에 접속하여 우측의 `Add to your Mbed Compiler`버튼을 클릭하여 사용할 디바이스를 추가합니다.

| Device | Link |
|:--------|:--------|
| FRDM-K64F | https://os.mbed.com/platforms/FRDM-K64F/ |

![][import1]

아래의 BG96_K6xF_pelion-example-frdm_Temp 링크에 접속하여 우측의 `Repository toolbox`의 `Import into Compiler`를 클릭하여 해당 예제를 추가합니다.


| Sample code | Link |
|:--------|:--------|
| BG96_K6xF_pelion-example-frdm_Temp | https://os.mbed.com/users/stkim92/code/BG96_K6xF_pelion-example-frdm_Temp/ |


![][import2]

### 2. Modify parameters
예제를 수행하기 위해서 별도의 Parameter가 수정될 필요는 없으나, 다른 센서를 추가하시려면 다음과 같은 부분을 수정해야 합니다.

**온도 데이터를 보내기 위한 Resouce 생성**
```cpp
MbedCloudClientResource *res_temperature; // Added
MbedCloudClientResource *res_magnometer_x;
MbedCloudClientResource *res_magnometer_y;
MbedCloudClientResource *res_magnometer_z;
```

**온도를 센싱하여 현재 온도 데이터를 업데이트**
```cpp
void sensors_update() {
 
    motion_data_counts_t acc_raw, mag_raw;
 
    sens_acc.getAxis(acc_raw);
 
    float mag_x = (double)mag_raw.x / 1000.0, mag_y = (double)mag_raw.y / 1000.0, mag_z = (double)mag_raw.z / 1000.0;
    float acc_x = (double)acc_raw.x / 1000.0, acc_y = (double)acc_raw.y / 1000.0, acc_z = (double)acc_raw.z / 1000.0;
    
    temp = tempVal.read_u16()/100;                 // Added
    voltage = temp * 4.95 / 1024;                  // Added
    celsius = getTemperature_C(voltage);           // Added
    printf("Celsius temp : %.2f C", celsius);      // Added
    if (endpointInfo) {                            // Added
        res_temperature->set_value(celsius);       // Added
    }                                              // Added
    
#ifdef TARGET_K66F
    float gyro_x = (double)sens_gyro.getX() / 1000.0, gyro_y = (double)sens_gyro.getY() / 1000.0, gyro_z = (double)sens_gyro.getZ() / 1000.0;
#endif /* TARGET_K66F */
 
    if (endpointInfo) {
#ifdef SEND_ALL_SENSORS
        res_temperature->set_value(celsius);       // Added
        res_accelerometer_x->set_value(acc_x);
        res_accelerometer_y->set_value(acc_y);
        res_accelerometer_z->set_value(acc_z);
    }
}
```

**센싱한 온도 데이터를 Pelion IoT Platform으로 전송**

```cpp
int main(void) {
    printf("\nStarting Simple Pelion Device Management Client example\n");
     ...
    }
     ...
    // Creating resources, which can be written or read from the cloud
    
    res_temperature = client.create_resource("3303/0/5700", "Temperature (C)");  // Added
    res_temperature->set_value(0);                                               // Added
    res_temperature->methods(M2MMethod::GET);                                    // Added
    res_temperature->observable(true);                                           // Added
    
    res_button = client.create_resource("3200/0/5501", "button_count");
    res_button->set_value(0);
    res_button->methods(M2MMethod::GET);
    eventQueue.dispatch_forever();
}
```



이 외에도 예제의 데이터 전송 주기를 변경하기 위해서는 `main.cpp` 파일의 45-45 line 을 원하는 주기로 변경하면 됩니다.
```cpp
// How often to fetch sensor data (in seconds)
#define SENSORS_POLL_INTERVAL 3.0
```

추가적으로, 해당 프로젝트에 인증서를 적용하기 위해 `Pelion Device Management` - `Appily update Certificate` 를 클릭합니다. 만약 기존에 인증서가 없다면 `Create`버튼을 눌러 인증서를 생성하여 적용합니다.

![][cert1]


### 3. Compile

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Demo

Mbed Pelion 인증서를 정상적으로 적용하여 예제를 실행하면 시리얼 터미널을 통해 다음과 같은 실행 결과를 볼 수 있습니다.

![][2]

Mbed Pelion IoT Platform에 접속하여 `Device directory` - `Devices`를 클릭하면 해당 디바이스의 State가 registered된 것을 확인 할 수 있으며, `/3303/0/5700` 리소스를 확인하면 현재 온도 데이터가 업데이트 되는 것을 확인 할 수 있습니다.

![][3]
![][4]


[mbed-getting-started]: ./mbed_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-frdm-k64f]: https://os.mbed.com/platforms/FRDM-K64F/
[link-mbed-bg96-pelion-example]: https://os.mbed.com/users/Daniel_Lee/code/BG96_K6xF_pelion-example-frdm/%29BG96_K6xF_pelion-example-frdm/

[import1]: ./imgs/mbed_guide_device_K64F_import.png
[import2]: ./imgs/mbed_guide_bg96_pelion_temp_import.png

[source1]: ./imgs/mbed_guide_bg96_pelion_temp_source-1.png
[source2]: ./imgs/mbed_guide_bg96_pelion_temp_source-2.png
[source3]: ./imgs/mbed_guide_bg96_pelion_temp_source-3.png

[cert1]: ./imgs/mbed_guide_pelion_temp_cert.png

[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_guide_pelion_iot_platform-1.png
[2]: ./imgs/mbed_guide_bg96_pelion_temp-1.png
[3]: ./imgs/mbed_guide_pelion_temp-2.png
[4]: ./imgs/mbed_guide_pelion_temp-3.png

