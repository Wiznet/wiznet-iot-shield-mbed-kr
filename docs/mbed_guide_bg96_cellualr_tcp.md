# MBED 기반의 Cat.M1 Cellular API를 이용한 TCP/IP 데이터 통신 가이드

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [예제 코드 빌드 및 실행](#Step-4-Build-and-Run)

<a name="Prerequisites"></a>
## 시작하기 전에

> * 하드웨어 설정과 개발환경 구축은 **[MBED 기반으로 Cat.M1 디바이스 개발 시작하기][mbed-getting-started]** 문서에 상세히 설명되어 있습니다.

> * Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]** 를 기준으로 작성되었습니다.

> * 본 예제는 아래 링크의 예제를 참고하여 제작하였으며, HW 구성의 경우 외장형 모뎀을 모듈로 변경하였고 SW의 경우 공인 IP 기반의 서버와 데이터를 송/수신할 수 있도록 일부 수정하였습니다.

>   * [mbed-os-example-cellular-BG96][link-mbed-bg96-cellular-example]

### Development Environment
* **[Arm MBED Web Compiler][link-mbed-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [FRDM-K64F][link-frdm-k64f] | WIoT-QC01 (BG96) |

<a name="Step-1-Overview"></a>
## 소개
본 문서는 Mbed-OS의 Cellular API 기반으로 지정된 서버에 TCP/UDP 소켓 연결을 하여 데이터를 송신하고, 서버로 부터 데이터를 수신하는 방법에 대한 가이드를 제공합니다.


![][1]

Mbed-OS의 Cellular API를 사용하지 않고 모듈의 AT 명령어를 이용하려면, 다음 링크를 참고하시기 바랍니다.

> [Mbed 기반의 Cat.M1 TCP/IP 데이터 통신 가이드](https://github.com/Wiznet/wiznet-iot-shield-mbed-kr/blob/master/docs/mbed_guide_bg96_tcp.md)


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
| mbed-os-example-cellular-BG96-TCP | https://os.mbed.com/users/stkim92/code/mbed-os-example-cellular-BG96-TCP/ |


![][import2]

### 2. Modify parameters
특정 서버로 데이터를 보내려면, 해당 서버의 IP 주소와 Port 번호를 수정해야 하며, 센서로 부터 수집한 데이터를 서버로 전송하려면 다음을 참고하여 보내는 데이터를 수정해야 합니다.

**서버 IP 주소 및 Post 번호 수정**
```cpp
NetworkInterface *iface;

// Echo server hostname
//nst char *host_name = MBED_CONF_APP_ECHO_SERVER_HOSTNAME;
const char *server_ip_address = "222.98.xxx.xxx";

// Echo server port (same for TCP and UDP)
//const int port = MBED_CONF_APP_ECHO_SERVER_PORT;
const int port = 7878;
```

**보내는 데이터 수정**
```cpp
ock.set_timeout(15000);
    int n = 0;
    const char *msg_string = "TEST";
```


### 3. Compile

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Demo

서버 IP와 Port 번호를 정상적으로 수정하여 예제를 실행하면 시리얼 터미널을 통해 다음과 같은 실행 결과를 볼 수 있습니다.

![][2]

해당 서버에서 디바이스가 보낸 데이터 'TEST'를 정상 수신한 것을 확인할 수 있으며, 디바이스가 데이터를 잘 수신하는 지 확인 하기 위해 '1234'라는 데이터를 디바이스로 보냅니다.

![][3]

디바이스가 서버로 부터 데이터를 정상 적으로 받으면 다음과 같이 수신 되었다는 메시지를 받고 프로그램이 종료되는 것을 확인 할 수 있습니다.

![][4]


[mbed-getting-started]: ./mbed_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-frdm-k64f]: https://os.mbed.com/platforms/FRDM-K64F/
[link-mbed-bg96-cellular-example]: https://os.mbed.com/users/Daniel_Lee/code/mbed-os-example-cellular-BG96/

[import1]: ./imgs/mbed_guide_device_K64F_import.png
[import2]: ./imgs/mbed_guide_bg96_cellular_tcp_import.png

[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_guide_bg96_cellular-1.png
[2]: ./imgs/mbed_guide_bg96_cellular-2.png
[3]: ./imgs/mbed_guide_bg96_cellular-3.png
[4]: ./imgs/mbed_guide_bg96_cellular-4.png

