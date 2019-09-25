# MBED 기반으로 Cat.M1 GPS 활용하기

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [AT 명령어](#Step-2-ATCommand)
-   [동작 구조 예제](#Step-3-SampleCode)
-   [예제 코드 빌드 및 실행](#Step-4-Build-and-Run)


<a name="Prerequisites"></a>
## 시작하기 전에

> * 하드웨어 설정과 개발환경 구축은 **[MBED 기반으로 Cat.M1 디바이스 개발 시작하기][mbed-getting-started]** 문서에 상세히 설명되어 있습니다.

> * Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]** 를 기준으로 작성되었습니다.


### Development Environment
* **[Arm MBED Web Compiler][link-mbed-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board | Accessory
|:--------:|:--------:|:--------:|
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-AM01 (AMM592) | External GPS Antenna |

> * 별도의 GNSS Antenna가 필요합니다.

<a name="Step-1-Overview"></a>
## 소개

본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo-L476RG 보드를 이용하여 Cat.M1 단말의 GPS 활용 방법에 대한 가이드를 제공합니다.

대부분의 Cat.M1 장치는 GPS(Global Positioning System)를 지원하고 있습니다. GPS는 범지구 위성 항법 시스템(GNSS, Global Navigation Satellite System)의 한 종류로, 대중적으로 활용되는 글로벌 위성 위치 확인 시스템입니다.

![][gnss]

Cat.M1 모듈의 GPS기능을 활용하면 아이나 반려동물, 귀중품의 위치를 실시간으로 파악하는 위치 트래커(GPS tracking unit) 등의 위치기반 서비스를 손쉽게 구현 할 수 있습니다. Cat.M1 기반의 위치 트래커는 기존의 블루투스 스마트 태그(Bluetooth Smart Tags)와 달리, 모듈 자체적으로 파악한 GPS 정보를 LTE망을 이용하여 사용자에게 전달함으로써 보다 빠르고 정확한 위치 파악이 가능한 것이 큰 장점입니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. GPS 정보의 획득 기능도 AT 명령어를 이용하여 활성화 할 수 있습니다.

<a name="Step-2-ATCommand"></a>
## AT 명령어


### 1. GPS 기능 활성화

GPS 측위는 부팅 완료 후 부터 AT command 로 사용 가능합니다.
GPS 기능 지원 모델 : **AMM592SK / AMM5400SKB / AMT5920SK**

**AT Command:** AT@AMGSTART

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT@AMGSTART| OK<br><br>@AMG: &lt;state code&gt;, [&lt;lat&gt;,&lt;lon&gt;] | AT@AMGSTART<br><br>OK |

**Defined values:**

| Parameter | 범위 | Description |
|:--------|:--------|:--------|
| &lt;state code&gt; | 0,1,2,3,4 | 0 : GPS positioning infomation<br>1 : Reference positioning information<br>2 : GPS positioning<br>3 : GPS error<br>4 : GPS time out |
|&lt;lat&gt;|0.000000 - 90.000000|latitude (위도)|
|&lt;lon&gt;|0.000000 - 90.000000|longitude (경도)|



### 2. GPS 기능 비활성화

GPS 기능을 비활성화 합니다.

**AT Command:** AT@AMGSTOP

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT@AMGSTOP | OK<br><br>@AMG: &lt;state code&gt; | AT@AMGSTOP <br> OK<br><br> @AMG:7|

**Defined values:**

| Parameter | 범위 | Description |
|:--------|:--------|:--------|
| &lt;state code&gt;| 6,7 | 6 : GPS engine off <br>7 : User ended the session |

### 3. GPS Session Type 설정

GPS 측위시의 Session Type를 설정합니다.

**AT Command:** AT@AMGST

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT@AMGST? | @AMGST:&lt;session_type&gt;<br>OK<br> | AT@AMGST? <br> <br> @AMGST:2<br><br>OK|
| Write | AT@AMGST=? | @AMGST:(range of session type) | AT@AMGSTOP=? <br> @AMGST:(0-2)<br><br>OK|
| Write | AT@AMGST=&lt;session_type&gt; | OK | AT@AMGST=2 <br> OK|

**Defined values:**

| Parameter | 범위 | Description |
|:--------|:--------|:--------|
| &lt;session_type&gt;| (0-2) | 0 : Latest (마지막 측위되었던 위치 표시) <br>1 : New (1회 측위) <br> 2: Tracking (연속 측위)|


<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

WIZnet IoT Shield를 Standalone 모드로 동작시켜, 터미널 프로그램으로 직접 AT 명령어를 입력해 보면서 동작 순서를 확인합니다.
기본적인 GPS 기능 활용을 위해서는 `AT@AMGST=1`, `AT@AMGSTART`(GPS on), `AT@AMGSTOP`(GPS off) AT 명령어를 이용합니다.

다음과 같이 확인 할 수 있습니다.

![][1]

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project

MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-AMM592-GPS | hhttps://os.mbed.com/users/vikshin/code/WIZnet-IoTShield-AMM592-GPS/ |


> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.

![][import1]

링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.

![][import2]

### 2. Modify parameters

GPS 예제는 별도로 수정할 부분이 없습니다.

### 3. Compile

상단 메뉴의 `Compile` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 GPS 서비스 운용에 대해 파악할 수 있습니다.
샘플 코드의 GPS관련 함수는 다음과 같습니다.

````cpp
int8_t setGpsSesssion_AMM592(int session_type);
````
* 파라메터에 따라 Session type setting을 수행합니다.


````cpp
int8_t setGpsOnOff_AMM592(bool onoff);
````
* 파라메터에 따라 GPS On / Off를 수행합니다.


````cpp
int8_t getGpsLocation_AMM592(gps_data *data);
````
* gps_data 구조체를 파라메터로 받아, Cat.M1 모듈로부터 획득한 GPS 정보를 저장하여 리턴합니다.




#### 4.1 Connect your board
WIZnet IoT shield + WIoT-AM01 인터페이스 보드와 ST Nucleo-L476RG 보드를 결합하여 PC와 USB로 연결합니다.
운영체제의 장치관리자를 통해 보드가 연결된 COM 포트를 확인 할 수 있습니다.

**GPS 안테나를 IoT shield 혹은 WIoT-AM01 인터페이스 보드에 연결합니다.**
> 실내에서는 안테나를 연결해도 위치 측위가 어려울 수 있습니다. 실외에서 테스트 해 보시기를 권장합니다.

#### 4.2 Set up serial terminal

보드와 통신을 위한 시리얼 터미널 프로그램을 준비합니다.

> 예제에서는 시리얼 모니터링을 위해 잘 알려진 Tera term 프로그램을 활용합니다.


Tera term 프로그램을 실행하여 이전 단계에서 확인된 보드의 COM 포트와 Baudrate 115200을 선택하여 시리얼 포트를 연결합니다.
이제 보드에서 출력하는 시리얼 메시지를 확인 할수 있습니다.

> 디버그 메시지 출력용 시리얼 포트 설정 정보: 115200-8-N-1, None

#### 4.3 Get GPS information through a serial terminal

성공적으로 연결이 완료된 후 보드를 리셋하면 다음과 같은 시리얼 출력을 확인 할 수 있습니다.
예제 코드는 Cat.M1 모듈의 상태를 확인한 후 GPS를 구동하여 정보를 출력하도록 구성되어 있습니다.

> GPS 위치 정보가 확인 될 때까지 약간의 시간이 필요할 수 있습니다.

![][2]

![][3]



#### 4.4 Check the location via Google Maps service (latitude, longitude)

GPS의 위치 정보 중 위도(latitude)와 경도(longitude)를 Google Maps에 입력하면 손쉽게 측정된 위치를 확인 할 수 있습니다.

> Google Maps: https://www.google.com/maps/

확인 결과, 분당구 수내동에 위치한 위즈네트 위치가 정확하게 표시되는 것을 알 수 있습니다.

![][4]







[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-gnss-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_GNSS_AT_Commands_Manual_V1.1.pdf

[mbed-getting-started]: ./mbed_get_started.md



[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_guide_bg96_gps-1.png
[2]: ./imgs/mbed_guide_bg96_gps-2.png
[3]: ./imgs/mbed_guide_bg96_gps-3.png
[4]: ./imgs/mbed_guide_bg96_gps-4.png
[gnss]: ./imgs/mbed_guide_bg96_gps-gnss.png
[usbport]: ./imgs/device_manager_quectel_usbport.png


