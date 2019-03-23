# MBED 기반으로 Cat.M1 디바이스 개발 시작하기

## 목차

-   [소개](#Introduction)
-   [Step 1: 필수 구성 요소](#Step-1-Prerequisites)
-   [Step 2: 디바이스 준비](#Step-2-PrepareDevice)
-   [Step 3: 예제 코드 빌드 및 실행](#Step-3-Build)
-   [더 보기](#ReadMore)


<a name="Introduction"></a>
## 소개

**문서의 주요 내용**

이 문서는 [Arm MBED](https://www.mbed.com/)를 기반으로 Cat.M1 장치의 개발 환경 구축 및 예제 코드 실행 과정에 대해 설명합니다.

각 과정에는 다음 내용들이 포함되어 있습니다:
- WIZnet IoT Shield와 ST Nucleo-L476RG 하드웨어 설정
- MBED 프로젝트 생성과 예제 코드 실행 과정
- 디바이스 상에서 Cat.M1 통신 기능 확인 과정


## Step 1: 필수 구성 요소
이 문서를 따라하기 전에 다음과 같은 것들이 준비되어야 합니다.

- **MBED 웹 컴파일러 개발환경을 활용 할 수 있는 컴퓨터**
  - 디버깅을 위한 시리얼 터미널 프로그램 ([Token2Shell](https://choung.net/token2shell), [PuTTY](https://www.putty.org), [TeraTerm](https://ttssh2.osdn.jp) 등)

- **Cat.M1 디바이스 개발용 하드웨어**
  - ST [Nucleo-L476RG](https://os.mbed.com/platforms/ST-Nucleo-L476RG/) (MBED enabled)
  - WIZnet IoT Shield
  - Cat.M1 Interface Board (택 1)
    - WIoT-QC01 (앰투앰넷 BG96)
    - WIoT-WM01 (우리넷 WM-N400MSE)
    - WIoT-AM01 (AM텔레콤 AMM5918K)

- **Cat.M1 모듈의 (시험 망)개통**
  - Cat.M1 모듈로 통신 기능을 구현하려면 **망 개통 과정**이 선행되어야 합니다.
    - 한국의 경우, 국내 Cat.M1 서비스 사업자인 SK Telecom의 망 개통 과정이 필요합니다.

> 모듈은 개발 단계에 따라 시험망 개통 - 상용망 개통 단계를 거쳐야 하며 외장형 모뎀은 즉시 상용망 개통이 가능합니다.
> - 개발 중인 제품의 시험망 개통인 경우 [SKT IoT OpenHouse](https://www.sktiot.com/iot/support/openhouse/reservation/openhouseMain)에 기술 지원 문의
> - 상용망 개통의 경우 USIM 구매 대리점이나 디바이스 구매처에 개통 문의

<a name="Step-2-PrepareDevice"></a>
## Step 2: 디바이스 준비

### 하드웨어 설정

WIZnet IoT Shield를 Nucleo-L476RG 하드웨어와 결합합니다. 
- 두 장치 모두 Arduino UNO Rev3 호환 핀 커넥터를 지원하므로 손쉽게 결합(Stacking) 할 수 있습니다.

![][hw-stack-nucleo]

IoT Shield는 다양한 밴더의 Cat.M1 모듈을 활용 할 수 있도록 하드웨어 설정을 제공합니다. 따라서 선택한 Cat.M1 인터페이스 보드를 확인하여 장치 설정이 필요합니다.

- 각각 밴더의 모듈은 동작 전압, PWRKEY 동작 등에 차이가 있습니다.
- 따라서 Jumper 설정을 통해 인터페이스 보드에 적합한 하드웨어 설정이 선행되어야 합니다.

MBED 기반에서 Nucleo-L476RG MCU 보드를 사용하는 경우 각 인터페이스 보드의 Jumper는 다음과 같이 설정합니다.

| WIoT-QC01 Jumpers<bR> | WIoT-WM01 Jumpers | WIoT-AM01 Jumpers |
|:--------------------------:|:--------------------------:|:--------------------------:|
|![][hw-settings-nucleo-qc01]|![][hw-settings-nucleo-wm01]|![][hw-settings-nucleo-am01]|

> 해당 설정은 각 모듈 별로 Nucleo 보드의 `D2`, `D8` 핀에서 지원하는 UART와 인터페이스 하기 위한 설정입니다. `D0`, `D1` 핀을 UART로 활용하거나 MCU 보드 없이 Standalone 모드로 활용하려는 경우 하드웨어 저장소의 **Settings**를 참고하시기 바랍니다.


### 디바이스 연결

하드웨어 설정 후 USB 커넥터를 이용하여 Nucleo-L476RG 보드와 PC를 연결합니다. PC 운영체제에서 보드와 연결된 COM 포트를 확인할 수 있습니다.
> 윈도우 운영체제의 경우, 장치 관리자(Device Manager)에서 COM 포트를 확인할 수 있습니다.

![][1]


> 장치 관리자에서 COM 포트를 확인할 수 없는 경우, 다음 링크에서 드라이버를 다운로드하여 설치하시기 바랍니다.
>  * [ST Link USB driver for Windows 7, 8, 10][st-usb-driver]



<a name="Step-3-Build"></a>
## Step 3: 예제 코드 빌드 및 실행

### MBED 프로젝트 생성 및 예제 코드 Import

MBED web compiler에 접속하여 Log in합니다. 만약 MBED 계정이 없을 경우 Sign up을 선택하여 계정을 생성할 수 있습니다.

> MBED web compiler: https://ide.mbed.com/compiler/

![][2]

접속에 성공하면 Workspace를 확인 할 수 있습니다. 
기존 사용자는 이전에 생성한 프로젝트들을 볼 수있으며, 처음 사용자의 경우 Program Workspace에 프로젝트가 없는 상태입니다.

![][3]

예제 코드 구동을 위해, 상단 메뉴의 `Import`를 클릭합니다. 


`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.

> 상단 `Click here` 를 클릭하시면 프로젝트 저장소의 Source URL로 **Import Program**이 가능합니다.


| Sample code | Repository Link |
|:--------|:--------|
| WIZnet-IoTShield-BG96-PING | https://os.mbed.com/users/hkjung/code/WIZnet-IoTShield-BG96-PING/ |


> WIZnet IoT Shield MBED 시작하기 가이드에서는 개발 환경 구축 및 예제 코드 빌드 실습을 위해 `WIZnet-IoTShield-BG96-PING` 프로젝트를 컴파일하고 실행합니다.

> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 **Program**으로 설정되어 있어야 합니다.

![][import1]

> 링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.

![][4]

### MBED 플랫폼 보드 선택

프로젝트 Import가 완료되면 하드웨어 플랫폼 보드를 선택합니다.
오른쪽 상단 버튼을 클릭하면 활성화되는 `Select a Platform` 팝업에서 보드를 선택 할 수 있습니다.

![][5]

![][6]

Add Board를 클릭하면 MBED 플랫폼 보드 리스트에서 활용할 보드를 선택할 수 있습니다.

> 플랫폼 보드를 추가하는 과정은 각 보드 당 한 번만 수행하면 됩니다. 최초 추가 과정 이후에는 다른 프로젝트에서도 별도의 보드 추가 과정 없이 기존에 추가한 플랫폼 보드를 선택 할 수 있습니다.

왼쪽 필터의 Target vendor - `STMicroelectonics`를 체크하여, ST 보드 중 본 가이드에서 활용할 `Nucleo-L476RG` 보드를 선택합니다.

![][7]

![][8]

오른쪽 중단의 `Add to your Mbed Compiler` 를 클릭하면 나의 MBED 플랫폼 보드 리스트에 `Nucleo-L476RG` 보드가 추가됩니다.

이제 `Select a Platform` 메뉴에서 [:heavy_check_mark: Select Platform] 버튼을 클릭하면 보드 선택이 완료됩니다.


![][9]


### 프로그램 컴파일

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

![][10]

컴파일이 성공적으로 완료되면 `project.boardname.bin` 파일이 생성되어 download 됩니다.

> Chrome 브라우저의 경우, 하단 다운로드 바에서 생성된 바이너리 파일을 확인할 수 있습니다.

> 예제 파일이 성공적으로 컴파일 되면 `WIZnet-IoTShield-BG96-Ping.NUCLEO_L476RG.bin`이 생성됩니다.

![][11]

이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

> MBED 플랫폼 보드는 `NODE_L746RG (F:)`와 같은 이름의 디스크 드라이브로 할당되어 있습니다. 이 곳에 생성된 펌웨어 바이너리 파일을 복사하면 됩니다.

### 시리얼 터미널 연결 및 실행

시리얼 터미널 프로그램을 실행하여 **디바이스 연결** 단계에서 확인한 보드의 COM 포트와 Baudrate 115200을 선택하여 시리얼 포트를 연결합니다.

> 디버그 메시지 출력용 시리얼 포트 설정 정보: 115200-8-N-1, None

펌웨어가 업데이트가 정상적으로 완료되면 다음과 같은 시리얼 출력을 확인할 수 있습니다.

* 지정된 두 곳의 목적지로 Ping 요청 및 응답을 수행합니다.
  * 첫번째 목적지: 8.8.8.8 (Google DNS server, IP 주소 접속 테스트)
  * 두번째 목적지: www.google.com (Google, Domain 접속 테스트)

![][12]



## 축하합니다
이제 여러분은 MBED 기반에서 WIZnet IoT Shield를 활용하여 LTE Cat.M1 응용을 개발할 모든 준비를 마쳤습니다!

<a name="ReadMore"></a>
## 더 보기
* [MBED 기반의 Cat.M1 TCP/IP 데이터 통신][mbed-guide-bg96-http]
* [MBED 기반의 Cat.M1 HTTP 활용 가이드][mbed-guide-bg96-http]
* [MBED 기반의 Cat.M1 MQTT 활용 가이드][mbed-guide-bg96-mqtt]




[st-usb-driver]: https://www.st.com/en/development-tools/stsw-link009.html

[mbed-guide-bg96-tcp]: ./mbed_guide_bg96_tcp.md
[mbed-guide-bg96-http]: ./mbed_guide_bg96_http.md
[mbed-guide-bg96-mqtt]: ./mbed_guide_bg96_mqtt.md

[hw-stack-nucleo]: ./imgs/hw/WIoT-Shield-QC01-Nucleo-L476RG.png

[hw-settings-nucleo-qc01]: ./imgs/hw/WIoT-QC01_JUMP_Arduino_serialD2_D8.png
[hw-settings-nucleo-wm01]: ./imgs/hw/WIoT-WM01_JUMP_Arduino_serialD2_D8.png
[hw-settings-nucleo-am01]: ./imgs/hw/WIoT-AM01_JUMP_Arduino_serialD2_D8.png

[import1]: ./imgs/mbed_guide_webide_import.png
[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_get_started-1.png
[2]: ./imgs/mbed_get_started-2.png
[3]: ./imgs/mbed_get_started-3.png
[4]: ./imgs/mbed_get_started-4.png
[5]: ./imgs/mbed_get_started-5.png
[6]: ./imgs/mbed_get_started-6.png
[7]: ./imgs/mbed_get_started-7.png
[8]: ./imgs/mbed_get_started-8.png
[9]: ./imgs/mbed_get_started-9.png
[10]: ./imgs/mbed_get_started-10.png
[11]: ./imgs/mbed_get_started-11.png
[12]: ./imgs/mbed_get_started-12.png

