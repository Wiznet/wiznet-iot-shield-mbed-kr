# MBED 기반으로 Cat.M1 PSM 설정하기

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

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-WM01 (WM-N400MSE) |

<a name="Step-1-Overview"></a>
## 소개

IoT 응용에서 배터리로 동작을 하는 디바이스의 경우, 디바이스가 소모하는 전류는 디바이스 수명과 직접적인 관계가 있으므로 소모 전류를 최소화 하는 것은 매우 중요합니다. 이와 같은 이유로 Cat.M1은 PSM과 Enhanced DRX(이하 eDRX)기술을 지원하여 소모하는 전류를 크게 줄일 수 있도록 합니다. PSM(Power Saving Mode)을 사용하면 디바이스의 Active/Sleep 상태를 조절하여 소모하는 전력을 줄일 수 있습니다.

본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo-L476RG 보드를 이용하여 Cat.M1 단말의 PSM 설정 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 PSM 설정 위한 통신 과정은 다음과 같은 순서로 구현합니다.

1. PSM 설정
2. PSM 설정 확인
3. 모듈 및 모뎀 Reboot

<a name="Step-2-ATCommand"></a>
## AT 명령어

> AT 명령어에 대한 좀 더 상세한 설명은 우리넷의 AT Command Manual에서 확인 하실 수 있습니다.
> * [WM-N400MSE_AT_Commands_Guide_v1.1][link-wm-n400mse-atcommand-manual]

### 1. PSM 설정

SM을 사용하기 위해 **AT+CPSMS** 명령어를 사용하며, 파라미터 설정을 통해 Active/Sleep 주기를 조절할 수 있습니다.
해당 명령의(Requested_Periodic-TAU)와 (Requested_Active-Time)의 시간을 설정함으로써 Active/Sleep 주기를 조절합니다. 

주기의 단위를 설정하기 위해 2초 / 30초 / 1분 / 10분 / 1시간 / 24시간 등을 6-8bits에서 설정하며 시간 값을 1-5bits에 설정합니다. 

예를 들어 (Requested_Periodic-TAU)를 1010011로 설정하였다면, 6-8bits가 101이므로 단위는 1분이며 1-2bit자리만 설정되었으므로 값은 6이고, 따라서 6분으로 설정한 것입니다.

**AT Command:** AT+CPSMS

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+CPSMS=[<Mode>[,(Requested_Periodic-RAU)[,(Requested_GPRS-READY-timer)[,(Requested_Periodic-TAU)[,(Requested_ActiveTime)]]]]] | OK<br><br>or<br><br>ERROR | AT+CPSMS=1,,,”10001010”,”00100100”<br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (Mode) | Integer | 0 : PSM 해제<br>1 : PSM 설정<br># MBED 기반으로 Cat.M1 PSM 설정하기

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (Mode) | Integer | 0 : PSM 해제<br>1 : PSM 설정<br>2 : PSM 해제 및 설정 초기화|
| (Requested_Periodic-RAU) | String | 000□□□□□ : (6-8 bits: 10분)   \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 10시간)  \* (1-5 bits: 이진수 값)<br>011□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>100□□□□□ : (6-8 bits: 30초)    \* (1-5 bits: 이진수 값)<br>101□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>110□□□□□ : (6-8 bits: 320시간) \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |
| (Requested_GPRS-READY-time) | String | 000□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |
| (Requested_Periodic-TAU) | String | 000□□□□□ : (6-8 bits: 10분)   \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 10시간)  \* (1-5 bits: 이진수 값)<br>011□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>100□□□□□ : (6-8 bits: 30초)    \* (1-5 bits: 이진수 값)<br>101□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>110□□□□□ : (6-8 bits: 320시간) \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |
| (Requested_Active-Time) | String | 000□□□□□ : (6-8 bits: 2초)     \* (1-5 bits: 이진수 값)<br>001□□□□□ : (6-8 bits: 1분)     \* (1-5 bits: 이진수 값)<br>010□□□□□ : (6-8 bits: 1시간)   \* (1-5 bits: 이진수 값)<br>111□□□□□ : 미설정<br> |

### 2. 최신 동기화 네트워크 시간 정보 획득

AT$$MSTIME 명령어는 모듈의 최근에 동기화 된 네트워크 시간 정보를 제공합니다.

예제에서 PSM 시간의 측정 시간 정보 출력을 위해 네트워크 시간 정보 획득 명령어를 활용합니다.

**AT Command:** AT$$MSTIME

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT$$MSTIME? | $$MSTIME:(mode),(year),(month),(day),(hour),(minute),(second)| AT$$MSTIME?<br>$$MSTIME:0,2017,12,01,10,12,34<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (mode) | Integer | 시간 설정 모드<br>0 : 자동 시간 설정<br>1 : 수동 시간 설정<br>|
| (year) | Integer | yyyy |
| (month) | Integer | mm |
| (day) | Integer | dd |
| (hour) | Integer | hh |
| (minute) | Integer | mm |
| (second) | Integer | ss |

<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

WIZnet IoT Shield를 Standalone 모드로 동작시켜, 터미널 프로그램으로 직접 AT 명령어를 입력해 보면서 동작 순서를 확인합니다.

```
// PSM 설정 (TAU time:300s, Active time:240s)
AT+CPSMS=1,,,"10100101","00100100"

OK

// 설정 적용을 위한 모듈 Reboot


```

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project

MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.

| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-WM-N400MSE-PSM | https://os.mbed.com/users/AustinKim/code/WIZnet-IoTShield-WM-N400MSE-PSM/ |

> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.

![][import1]

링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.

![][import2]

### 2. Modify parameters

PSM 예제는 별도로 수정할 부분이 없습니다.

> [주의] TAU 시간은 SKTelecom 기준으로 최소 5분까지만 지원됩니다. 5분 이하의 시간을 입력할 경우 SKTelecom 망에서 지원하지 않아 PSM 진입 후 Booting하지 않을 수 있습니다.

### 3. Compile

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 PSM 설정 방법과 활용 방법에 대해 파악할 수 있습니다.
샘플 코드의 PSM관련 함수는 다음과 같습니다.

```cpp
int8_t setPsmActivate_WM01(char *Requested_Periodic_TAU, char *Requested_Active_Time);
```
* PSM TAU와 Active time의 설정 및 기능 활성화를 수행합니다.
* 파라메터는 "10010101", "00100100" 형태의 문자열입니다.

```cpp
int8_t setPsmDeactivate_WM01(void);
```
* PSM 기능을 비활성화 하는 함수입니다.

샘플 코드의 네트워크 시간 정보 관련 함수는 다음과 같습니다.

```cpp
int8_t getNetworkTimeLocal_WM01(char *time);
```
* 파라메터를 통해 네트워크 동기화 시간을 기반으로 계산된 Local 시간을 문자열로 제공하는 함수입니다.
* 한국의 경우 GMT+09가 적용됩니다.

#### 4.1 Connect your board
WIZnet IoT shield + WIoT-WM01 인터페이스 보드와 ST Nucleo-L476RG 보드를 결합하여 PC와 USB로 연결합니다.
운영체제의 장치관리자를 통해 보드가 연결된 COM 포트를 확인 할 수 있습니다.

#### 4.2 Set up serial terminal

보드와 통신을 위한 시리얼 터미널 프로그램을 준비합니다.

> 예제에서는 시리얼 모니터링을 위해 잘 알려진 Tera term 프로그램을 활용합니다.


Tera term 프로그램을 실행하여 이전 단계에서 확인된 보드의 COM 포트와 Baudrate 115200을 선택하여 시리얼 포트를 연결합니다.
이제 보드에서 출력하는 시리얼 메시지를 확인 할수 있습니다.

> 디버그 메시지 출력용 시리얼 포트 설정 정보: 115200-8-N-1, None

#### 4.3 PSM(Power Saving Mode) Demonstration

성공적으로 연결이 완료된 후 보드를 리셋하면 시리얼 출력을 확인 할 수 있습니다.
예제 코드는 Cat.M1 모듈의 상태를 확인한 후 PSM 설정 여부에 따라 PSM 설정 및 재시작 후 동작을 확인하도록 구성되어 있습니다.



[mbed-getting-started]: ./mbed_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-wm-n400mse-atcommand-manual]: ./datasheet/WM-N400MSE_AT_Commands_Guide_v1.1.pdf

[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_guide_bg96_psm-1.png
[2]: ./imgs/mbed_guide_bg96_psm-2.png


