# MBED 기반의 Cat.M1 TCP/IP 데이터 통신 가이드

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [AT 명령어](#Step-2-ATCommand)
-   [동작 구조 예제](#Step-3-SampleCode)

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
본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo-L476RG 보드를 이용하여 Cat.M1 단말의 TCP 데이터 송수신 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 TCP clint(UDP 포함)의 통신 과정은 다음과 같은 순서로 구현합니다.

1. 네트워크 인터페이스 활성화
2. 소켓 열기 - 목적지 IP 주소 및 포트번호 포함
3. 데이터 전송 - 송신 및 수신
4. 소켓 닫기
5. 네트워크 인터페이스 비활성화

추가적으로, TCP 가이드 문서에는 다른 응용 가이드 문서에는 포함되어 있지 않은 Cat.M1 단말의 상태 확인 및 PDP context 관련 명령어에 대한 내용이 함께 포함되어 있습니다. 해당 명령어는 응용 구현 시 필수적으로 활용되어야 하므로, 함께 확인하시기 바랍니다.
* Echo 모드 설정: `ATE`
* USIM 상태 확인: `AT$$STAT?`
* 망 상태 점검: `AT+CEREG?`
* PDP Context 활성화 및 비활성화: `AT*RNDISDATA`


<a name="Step-2-ATCommand"></a>
## AT 명령어

> AT 명령어에 대한 좀 더 상세한 설명은 Quectel AT Command Manual에서 확인 하실 수 있습니다.
> * [WM-N400MSE_AT_Commands_Guide_v1.1][link-wm-n400mse-atcommand-manual]

### 1. Echo 모드 설정

ATE0로 설정되면 입력된 명령어 Echo back이 비활성화 됩니다.
MCU board로 Cat.M1 모듈을 제어하는 경우 해당 명령어를 사용합니다.

**AT Command:** ATE

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | ATE(value1) | OK | ATE0<br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value1) | integer | 0 : Echo mode OFF<br>1 : Echo mode ON |

### 2. USIM 상태 확인
이 명령어는 USIM의 Password를 입력하거나 password 입력이 필요 없는 경우 USIM의 정상 운용이 가능한 상황인지 확인합니다. 본 가이드에서는 password가 없는 상황에서 USIM 상태를 확인하기 위해 사용합니다.
> **READY** 응답이 출력되면 정상입니다.

**AT Command:** AT$$STAT?

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT$$STAT? | $$STAT:(value1) | AT$$STAT?<br>$$STAT:READY<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value1) | string | INSERT : USIM card inserted<br>OPEN : MSISDN NULL<br>READY : 정상적으로 Card Initialization 마친 상태<br>MNCCARD : 다른 사업자 USIM<br>MCCRAD : 해외 사업자 USIM<br>TESTCARD : 장비 테스트 USIM<br>ONCHIP : onChip SIM mode<br>PIN : SIM PIN, SIM PUK 등 남은 시도 횟수<br>NET PIN : PLMN ID 이외의 값을 가진 카드<br>SIM PERM BLOCK : PUK 모두 실패. 카드 교체 필요<br>SIM PIN VERIFIED : PIN code 입력 성공<br>SIM PERSO OK : Personalization unlock 성공<br>FAILURE, REMOVED : USIM removed<br>FAILURE,NO_CARD : USIM 삽입 안됨<br>AILURE,ERROR : USIM인식 Error |

### 3. 망 상태 점검

망 서비스 상태 확인을 위해 사용되는 명령어 입니다. 디바이스 구현 시, 망 연결 유지를 위해 주기적으로 체크하는 것을 권장합니다.

**AT Command:** AT+CEREG?

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT+CEREG? | +CEREG: (value1),(value2)<br><br>OK | AT+CEREG?<br>+CEREG: 0,1<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------:|:--------|
| (value1) | integer | 0 : Disable network registration unsolicited result code<br>1 : Enable network registration unsolicited result code<br>2 : Enable network registration and location information unsolicited result code<br>4 : For a UE that wants to apply PSM, enable network registration and location information unsolicited result code |
| (value2) | integer | 0 : Not registered. MT is not currently searching an operator to register to.<br>1 : Registered, home network<br>2 : Not registered, but MT is currently trying to attach or searching an operator to register to.<br>3 : Registration denied<br>4 : Unknown<br>5 : Registered, roaming |

### 4. PDP Context 활성화 및 비활성화
> PDP(Packet Data Protocol)란 단말과 외부 패킷 데이터 네트워크 사이의 데이터 송수신을 위한 연결을 제공하기 위해 사용하는 네트워크 프로토콜을 뜻하며, PDP Context는 이러한 연결 과정에서 사용되는 정보의 집합을 의미합니다.

**AT Command:** AT*RNDISDATA

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT*RNDISDATA=(value1) | *RNDISDATA=(value2)<br><br>OK | AT\*RNDISDATA=1<br>*RNDISDATA:1<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value1) | integer | 0 : RNDIS Device 미사용(전화 접속 연결 사용)<br>1 : RNDIS Device 사용(전화 접속 연결 사용 불가)|

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value2) | integer | 0 : RNDIS Device 미사용(전화 접속 연결 사용)<br>1 : RNDIS Device 사용(전화 접속 연결 사용 불가)|

### 5. 소켓 생성
소켓 서비스를 생성하는 명령어 입니다.

**AT Command:** AT+WSOCR

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+WSOCR=(value1),(value2),(value3),(value4),(value5) | +WSOCR:(value6),(value7),(value8),(value9)<br><br>OK | AT+WSOCR=0,222.98.173.214,8080,1,0<br>+WSOCR:1,0,64:ff9b::222.98.173.214/8080,TCP<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value1) | integer | Socket ID |
| (value2) | string | IP Address or URL |
| (value3) | integer | Port |
| (value4) | integer | Protocol<br>1 : TCP<br>2 : UDP |
| (value5) | integer | Packet Type<br>0 : ASCII<br>1 : HEX<br>2 : Binary |

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value6) | integer | Result<br>0 : 실패<br>1 : 성공 |
| (value7) | integer | Socket ID |
| (value8) | string | IP Adress/Port |
| (value9) | integer | Protocol<br>1 : TCP<br>2 : UDP |

### 6. 소켓 연결
지정된 소켓 서비스를 연결하는 명령어 입니다.

**AT Command:** AT+WSOCO

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+WSOCO=(value1)| +WSOCO:(value2),(value3),OPEN_WAIT<br><br>OK<br>+WSOCO:(value4),OPEN_CMPL | AT+WSOCO=0<br>+WSOCO:1,0,OPEN_WAIT<br><br>OK<br>+WSOCO:0,OPEN_CMPL |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value1) | integer | Socket ID |

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value2) | integer | Result<br>0 : 실패<br>1 : 성공 |
| (value3) | integer | Socket ID |
| (value4) | integer | Socket ID |

### 7. 소켓 데이터 전송

지정된 소켓으로 데이터를 전송하는 명령어 입니다.

**AT Command:** AT+WSOWR

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+WSOWR=(value1),(value2),(value3) | +WSOWR:(value4),(value5)<br><br>OK | AT+WSOWR=0,12,Hello Cat.M1<br>+WSOWR:1,0<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value1) | integer | Socket ID |
| (value2) | integer | Data Length |
| (value3) | string | Data |

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value4) | integer | Result<br>0 : 실패<br>1 : 성공 |
| (value5) | integer | Socket ID |

### 8. 소켓 데이터 수신
지정된 소켓으로부터 데이터를 수신하는 명령어 입니다.

**AT Command:** +WSORD

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | | +WSORD=(value1),(value2),(value3) | +WSORD:0,9,Hi Cat.M1 |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value1) | integer | Socket ID |
| (value2) | integer | Data Length |
| (value3) | string | Data |

### 9. 소켓 종료
지정된 소켓 서비스를 종료하는 명령어 입니다.

**AT Command:** AT+WSOCL

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+WSOCL=(value1) | +WSOCL:(value2),(value3),CLOSE_WAIT<br><br>OK<br>+WSOCL:(value4),CLOSE_CMPL | AT+WSOCL=0<br>+WSOCL:1,0,CLOSE_WAIT<br><br>OK<br>+WSOCL:0,CLOSE_CMPL |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value1) | integer | Socket ID |

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value2) | integer | Result<br>0 : 실패<br>1 : 성공 |
| (value3) | integer | Socket ID |
| (value4) | integer | Socket ID |

<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

```
// AT 명령어 echo 비활성화
ATE0
OK

// USIM 상태 확인 (READY : 정상)
AT$$STAT?
$$STAT:READY

OK

// 망 접속 확인
AT+CEREG?
+CEREG: 0,1

OK

// PDP context 활성화
AT*RNDISDATA=1
*RNDISDATA:1

OK

// TCP socket 생성 (목적지 IP 주소 및 Port number)
AT+WSOCR=0,222.98.173.214,8080,1,0
+WSOCR:1,0,64:ff9b::222.98.173.214/8080,TCP

OK

// TCP socket 연결
AT+WSOCO=0
+WSOCO:1,0,OPEN_WAIT

OK
+WSOCO:0,OPEN_CMPL

// TCP data 송신
AT+WSOWR=0,12,Hello Cat.M1
+WSOWR:1,0

OK

// TCP data 수신
+WSORD:0,9,Hi Cat.M1

// TCP socket 종료
AT+WSOCL=0
+WSOCL:1,0,CLOSE_WAIT

OK
+WSOCL:0,CLOSE_CMPL

// PDP context 비활성화
AT*RNDISDATA=0
*RNDISDATA:0

OK
```

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project

MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-WM-N400MSE-TCP | https://os.mbed.com/users/AustinKim/code/WIZnet-IoTShield-WM-N400MSE-TCP/ |


> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.

![][import1]

링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.

![][import2]

### 2. Modify parameters

TCP/IP 데이터 통신 가이드 예제 코드의 동작을 위해서 Cat.M1 장치가 접속할 TCP server 운용 PC의 IP 주소 및 Port 정보를 수정해야 합니다.
Main.cpp 파일 상단의 다음 부분을 목적지 주소로 변경 하시기 바랍니다.

````cpp
// Destination (Remote Host)
// IP address and Port number
char dest_ip[] = "222.xxx.xxx.xxx";
int  dest_port = 8000;
````

### 3. Compile

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 TCP 소켓 서비스 운용에 대해 파악할 수 있습니다.

샘플 코드는 아래와 같은 순서로 동작합니다.

* 하드웨어 초기화 과정
  * 시리얼 인터페이스 활성화 및 Parser 초기화
  
* Cat.M1 모듈 상태 확인
  * USIM 상태 확인
  * 망 등록 상태 점검
  
* PDP context 활성화

* TCP client 동작
  * 소켓 오픈 및 목적지(IP/Port) 연결
  * 데이터 송신 및 수신

> Cat.M1 모듈의 테스트를 위해 TCP client sample을 활용 할 경우, 사용자의 네트워크 구성에 따라 공유기(router)의 [포트 포워딩(Port forwarding)](https://opentutorials.org/course/3265/20038) 기능이 필요할 수 있습니다.
>  * TCP server가 될 PC가 192.168.1.xxx 등의 사설 IP 주소 대역을 할당 받아 운용되고 있는 경우


#### 4.1 Connect your board
WIZnet IoT shield + WIoT-WM01 인터페이스 보드와 ST Nucleo-L476RG 보드를 결합하여 PC와 USB로 연결합니다.
운영체제의 장치관리자를 통해 보드가 연결된 COM 포트를 확인 할 수 있습니다.

#### 4.2 Set up serial terminal and TCP server

보드와 통신을 위한 시리얼 터미널 프로그램과 TCP server 프로그램을 준비합니다.

> 예제에서는 시리얼 모니터링을 위해 잘 알려진 Tera term 프로그램과 ezTerm 프로그램을 활용합니다.


Tera term 프로그램을 실행하여 이전 단계에서 확인된 보드의 COM 포트와 Baudrate 115200을 선택하여 시리얼 포트를 연결합니다.
이제 보드에서 출력하는 시리얼 메시지를 확인 할수 있습니다.

> 디버그 메시지 출력용 시리얼 포트 설정 정보: 115200-8-N-1, None

ezTerm 프로그램에세 `TCP 서버`를 선택하고 로컬 포트를 입력 후, `접속 대기` 버튼을 누르면 TCP server 시작됩니다.

![][1]



#### 4.3 Send a message from device to teminal (Send)

성공적으로 연결이 완료된 후 보드를 리셋하면 다음과 같은 시리얼 출력을 확인 할 수 있습니다.
예제 코드는 Cat.M1 모듈의 상태를 확인한 후 TCP client로 동작하여 지정된 목적지로 `Hello Cat.M1` 메시지를 전송합니다.

TCP server에서 Cat.M1 모듈이 전송한 메시지를 확인할 수 있습니다.

![][2]

![][3]

#### 4.3 Send a message from terminal to device (Receive)

예제 코드는 TCP server 측에서 보내온 메시지가 Cat.M1 모듈로 정상 수신 되면 시리얼 디버그 포트를 통해 출력하도록 구성되어 있습니다.
보내진 `Hi Cat.M1 Test01`, `Hi Cat.M1 Test02` 메시지가 시리얼 포트로 출력되는 것을 확인 할 수 있습니다.

> 예제 코드는 `exit`를 수신하면 소켓 연결을 종료하고 프로그램을 마치도록 구현되어 있습니다.

![][4]

![][5]

![][6]

![][7]



[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-wm-n400mse-atcommand-manual]: ./datasheet/WM-N400MSE_AT_Commands_Guide_v1.1.pdf

[mbed-getting-started]: ./mbed_get_started.md
[mbed-guide-bg96-http]: ./mbed_guide_bg96_http.md
[mbed-guide-bg96-mqtt]: ./mbed_guide_bg96_mqtt.md

[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_guide_wm-n400mse_tcp-1.png
[2]: ./imgs/mbed_guide_wm-n400mse_tcp-7.png
[3]: ./imgs/mbed_guide_wm-n400mse_tcp-2.png
[4]: ./imgs/mbed_guide_wm-n400mse_tcp-2.png
[5]: ./imgs/mbed_guide_wm-n400mse_tcp-3.png
[6]: ./imgs/mbed_guide_wm-n400mse_tcp-4.png
[7]: ./imgs/mbed_guide_wm-n400mse_tcp-8.png