# MBED 기반의 Cat.M1 TCP/IP 데이터 통신 가이드

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [AT 명령어](#Step-2-ATCommand)
-   [동작 구조 예제](#Step-3-SampleCode)
-   [예제 코드 빌드 및 실행](#Step-4-Build-and-Run)
-   [더 보기](#ReadMore)

<a name="Prerequisites"></a>
## 시작하기 전에

> * 하드웨어 설정과 개발환경 구축은 **[MBED 기반으로 Cat.M1 디바이스 개발 시작하기][mbed-getting-started]** 문서에 상세히 설명되어 있습니다.

> * Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]** 를 기준으로 작성되었습니다.


### Development Environment
* **[Arm MBED Web Compiler][link-mbed-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-QC01 (BG96) |

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
* USIM 상태 확인: `AT+CPIN?`
* 망 등록 및 상태 점검: `AT+QCDS`
* PDP Context 활성화 및 비활성화: `AT+QIACT, AT+QIDEACT`


<a name="Step-2-ATCommand"></a>
## AT 명령어

> AT 명령어에 대한 좀 더 상세한 설명은 Quectel AT Command Manual에서 확인 하실 수 있습니다.
> * [BG96_TCP/IP_AT_Commands_Manual_V1.0][link-bg96-tcp-manual]
> * [BG96_AT_Commands_Manual_V2.1][link-bg96-atcommand-manual]

### 1. Echo 모드 설정

ATE0로 설정되면 입력된 명령어 Echo back이 비활성화 됩니다.
MCU board로 Cat.M1 모듈을 제어하는 경우 해당 명령어를 사용합니다.

**AT Command:** ATE

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | ATE(value) | OK | ATE0<br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value) | Integer | 0 : Echo mode OFF<br>1 : Echo mode ON |

### 2. USIM 상태 확인
이 명령어는 USIM의 Password를 입력하거나 password 입력이 필요 없는 경우 USIM의 정상 운용이 가능한 상황인지 확인합니다. 본 가이드에서는 password가 없는 상황에서 USIM 상태를 확인하기 위해 사용합니다.
> **READY** 응답이 출력되면 정상입니다.

**AT Command:** AT+CPIN?

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT+CPIN? | +CPIN: (code) | AT+CPIN?<br>+CPIN: READY<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (code) | String | **READY**: MT is not pending for any password<br>SIM PIN: MT is waiting for (U)SIM PIN to be given<br>SIM PUK: MT is waiting for (U)SIM PUK to be given<br>SIM PIN2: MT is waiting for (U)SIM PIN2 to be given<br>SIM PUK2: MT is waiting for (U)SIM PUK2 to be given<br>PH-NET PIN: MT is waiting for network personalization password to be given<br>PH-NET PUK: MT is waiting for network personalization unblocking password |

### 3. 망 등록 및 상태 점검

망 서비스 상태 확인을 위해 사용되는 명령어 입니다. 디바이스 구현 시, 망 연결 유지를 위해 주기적으로 체크하는 것을 권장합니다.

> 일반적으로 "SRV" 상태에 RSRP -120 이상이면 통신이 가능한 것으로 판단할 수 있습니다.

**AT Command:** AT+QCDS

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT+QCDS | +QCDS: (status),"CAT-M",(PLMN),(cell_id),(channel),(type),(service_domain),(cell_id_p),(tac),(RSSI),(RSRP),(RSRQ),(TMSI),(RI),(CQI) | AT+QCDS<br>+QCDS: "SRV","CAT-M",45012,2500,13BD0B,R13,3,368,2058,-31,-55,-6,3,128,0,0,0<br><bR>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------:|:--------|
| (status) | String | 서비스 상태 표시<br>"SRV": 서비스 원활<br>"LIMITED": 서비스 제한 |
| (PLMN) | Integer | 망 서비스 사업자의 네트워크 식별 번호<br>00000: UNKNOWN<br>45012: SK Telecom CAT-M |
| (cell_id) | Hex | Cell ID |
| (channel) | Hex | Channel |
| (type) | String | 네트워크 타입 |
| (service_domain) | Integer | 서비스 도메인<br>(3 = CS + PS)
| (cell_id_p) | Integer | Physical Cell ID |
| (tac) | Integer | TAC |
| (RSSI) | Integer | 수신 Signal Power의 총 크기(dBm) |
| (RSRP) | Integer | 수신 Reference Signal의 Power(dBm)<br>-140 이하이면 NO SERVICE<br>-120 이하이면 약전계<br>-90 ~ -119 이면 NORMAL<br>-70 ~ -89 이면 GOOD<br>-70 이상이면 VERY GOOD |
| (RSRQ) | Integer | 수신 Power 대비 Reference 신호의 비(dBm)<br>-20 이하이면 데이터 접속 어려움<br>-15 ~ -19 이면 NORMAL<br>-10 ~ -14 이면 GOOD<br>-9 이상이면 VERY GOOD |
| (TMSI) | - | Temporary Mobile Subscriber Identity |
| (RI) | - | Rank Indicator |
| (CQI) | - | Channel Quality Indicator |

### 4. PDP Context 활성화
> PDP(Packet Data Protocol)란 단말과 외부 패킷 데이터 네트워크 사이의 데이터 송수신을 위한 연결을 제공하기 위해 사용하는 네트워크 프로토콜을 뜻하며, PDP Context는 이러한 연결 과정에서 사용되는 정보의 집합을 의미합니다.

**AT Command:** AT+QIACT

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QIACT=? | +QIACT: (1-16)<br><br>OK | - |
| Read | AT+QIACT? | +QIACT:<br>(1,(context_state),(context_type)[,(IP_address)]<br>[.....<br>+QIACT:<br>(16,(context_state),(context_type)[,(IP_address)]]<br><br>OK | AT+QIACT?<br><br>+QIACT:<br> 1,1,2,"2001:2D8:13B1:4A65:0:0:A248:8002"<br><br> OK |
| Write | AT+QIACT=(contextID) | 지정된 ID의 Context를 활성화:<br>OK or ERROR | AT+QIACT=1<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (contextID) | Integer | Context ID (범위: 1-16) |
| (context_state) | Integer | Context의 상태<br>0 : Deactivated<br>1 : Activated |
| (context_type) | Integer | Protocol 타입<br>1 : IPv4<br>2 : IPv6 |
| (IP_address) | String | Context가 활성화되어 부여된 Local IP 주소 |

### 5. PDP Context 비활성화

**AT Command:** AT+QIDEACT

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QIDEACT=? | +QIDEACT: (1-16)<br><br>OK | - |
| Write | AT+QIDEACT=(contextID) | 지정된 ID의 Context를 비활성화:<br>OK or ERROR | AT+QIDEACT=1<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (contextID) | Integer | Context ID (범위: 1-16) |


### 6. 소켓 열기

소켓 서비스를 오픈하는 명령어 입니다.
> 전송 프로토콜 타입(TCP or UDP) 및 목적지 IP 주소와 포트번호를 포함하며, TCP의 경우 소켓 오픈 및 연결(connect) 시도가 수행됩니다. (buffer access 모드와 direct push 모드는 연결 성공 시 +QIOPEN: 0,0 출력, transparent access 모드의 경우는 CONNECT 출력)

**AT Command:** AT+QIOPEN

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+QIOPEN=(contextID),(connectID),(service_type),(IP_address)/(domain_name),(remote_port)[,(local_port)[,(access_mode)]] | transparent access 모드에서 성공적으로 소켓 서비스가 오픈된 경우:<br>CONNECT<br>실패한 경우:<br>ERROR<br><br>buffer access 모드 혹은 direct push 모드는 다음과 같이 출력:<br>OK<br><br>+QIOPEN: (connectID),(err)<br><br> *(err)이 0인 경우 소켓 서비스 오픈 성공 | AT+QIOPEN=1,0,"TCP","222.98.xxx.xxx",50001<br><br>OK<br>+QIOPEN: 0,0 |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (contextID) | Integer | Context ID (범위: 1-16) |
| (connectID) | Integer | 소켓 서비스 인덱스 (범위: 0-11) |
| (service_type) | String | 소켓 서비스 타입<br>"TCP": TCP client로 connection 수행<br>"UDP": UDP client로 connection 수행<br>"TCP LISTENER": TCP server로 client의 connection 대기 수행(listen)<br>"UDP SERVICE": UDP 서비스 시작 |
| (IP_address) | String | (service_type)이 TCP 혹은 UDP인 경우, 목적지(destination)의 IP 주소를 입력<br>(service_type)이 TCP LISTENER 혹은 UDP SERVICE인 경우, "127.0.0.1"로 고정 입력 |
| (domain_name) | String | 접속할 목적지(destination = remote server)의 Domain name |
| (remote_port) | Integer | 접속할 목적지의 포트 번호 (범위: 0-65535)<br>(service_type)이 TCP 혹은 UDP 인 경우에만 유효함  |
| (local_port) | Integer | 로컬 포트 번호<br>(service_type)이 TCP LISTENER 혹은 UDP SERVICE인 경우 응용에 맞춰 꼭 명세되어야 함<br>(service_type)이 TCP 혹은 UDP인 경우, 0으로 입력하면 자동으로 할당 (수동 입력도 가능)|
| (access_mode) | Integer | 소켓 서비스의 data access 모드<br>0: Buffer access mode<br>1: Direct push mode<br>2: Transparent access mode |
| (err) | Integer | 에러 코드<br>상세 내용은 [BG96_AT_Commands_Manual: Chapter 4][link-bg96-atcommand-manual] 참조 |

### 7. 소켓 종료

지정된 소켓 서비스를 종료하는 명령어 입니다.

**AT Command:** AT+QICLOSE

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+QICLOSE=(connectID)[,(timeout)] | 성공적으로 Close 된 경우<br>OK<br><br>Close에 실패하는 경우<br>ERROR | AT+QICLOSE=0<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (connectID) | Integer | 소켓 서비스 인덱스 (범위: 0-11) |
| (timeout) | Integer | 지정된 (timeout) 시간 안에 FIN ACK가 수신되지 않으면 모듈은 강제로 소켓을 Close함 <br>(범위: 1-65535) (초기값: 10) (단위: second) |

### 8. 소켓 데이터 전송

지정된 소켓으로 데이터를 전송하는 명령어 입니다.
> "TCP" 모드 외의 (service_type)에 대한 명령어는 [BG96_TCP/IP_AT_Commands_Manual][link-bg96-tcp-manual]을 참고하시기 바랍니다.

**AT Command:** AT+QISEND

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | (service_type) "TCP", "UDP", "TCP INCOMING" 만 해당<br><br>AT+QISEND=(connectID),(send_length)<br>> 응답이 수신되면 (send_length)만큼의 데이터 전송 | > <br>(지정한 길이만큼 데이터 전송)<br><br>연결 상태에서 성공적으로 데이터 전송한 경우<br>SEND OK<br><br>연결 상태이나 데이터 전송이 실패한 경우<br>SEND FAIL<br><br>연결 상태가 아니거나, 비정상적으로 close 되거나, parameter가 정상적이지 않은 경우<br>ERROR | AT+QISEND=0,10<br>> helloworld<br><br>SEND OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (connectID) | Integer | 소켓 서비스 인덱스 (범위: 0-11) |
| (send_length) | Integer | 전송할 데이터의 길이 (최대 1460 byte) |



### 9. 소켓 데이터 수신

지정된 소켓의 버퍼로 데이터가 수신 될 경우, 모듈은 `+QIURC: "recv",<connectID>` 명령을 출력하며, 이 때 AT+QIRD 명령을 이용하면 수신 데이터를 확인할 수 있습니다.

> "+QIURC" 이후 수신 버퍼에 추가 데이터 수신이 발생해도 더 이상의 알림을 출력하지 않습니다. 다음 알림 명령은 버퍼가 비워진 이후에 출력됩니다.

> "TCP" 모드 외의 (service_type)에 대한 명령어는 [BG96_TCP/IP_AT_Commands_Manual][link-bg96-tcp-manual]을 참고하시기 바랍니다.


| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | (service_type) "TCP", "UDP", "TCP INCOMING" 만 해당<br><br>AT+QIRD=(connectID),[(read_length)]<br>| 지정한 연결에 수신된 데이터가 있을 경우<br>+QIRD: (read_actual_length)(CR)(LF)(data)<br><br>OK<br><br>만약 수신된 데이터가 없을 경우<BR>+QIRD: 0<br><br>OK<br><br>연결 상태가 아닌 경우<br>ERROR| (+QIURC: "recv",0 출력 시) AT+QIRD=0<br><br>+QIRD: 10<br>helloworld<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (connectID) | Integer | 소켓 서비스 인덱스 (범위: 0-11) |
| (read_length) | Integer | 수신 할 데이터의 길이 (0-1500, byte) |
| (read_actual_length) | Integer | 수신 데이터의 실제 길이 (byte) |
| (data) | String | 수신한 데이터 |



<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

```
/* BG96 TCP Send/Recv */

// AT 명령어 echo 비활성화
ATE0

// USIM 상태 확인 (READY면 정상)
AT+CPIN?

// 망 접속 확인 (SRV면 접속)
AT+QCDS

// PDP context 활성화
AT+QIACT=1

// PDP context 활성화 확인
AT+QIACT?

// TCP socket open (목적지 IP주소 및 포트번호)
AT+QIOPEN=1,0,"TCP","222.98.xxx.xxx",50001

// TCP Send
AT+QISEND=0,4
> "ABCD"
or
AT+QISENDEX=0,"41424344" // Hex only

// TCP Receive (+QIURC: "recv",0 수신 시 수행)
AT+QIRD=0

// TCP socket close
AT+QICLOSE=0

// TCP socket close 여부 확인
AT+QISTATE?

// TCP context 비활성화
AT+QIDEACT=1

// TCP/IP context 비활성화 확인
AT+QIACT?

```

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project

MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-BG96-TCP | https://os.mbed.com/users/hkjung/code/WIZnet-IoTShield-BG96-TCP/ |


> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.

![][import1]

링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.

![][import2]

### 2. Modify parameters

TCP/IP 데이터 통신 가이드 예제 코드의 동작을 위해서 Cat.M1 장치가 접속할 TCP server 운용 PC의 IP 주소 및 Port 정보를 수정해야 합니다.
Main.cpp 파일 상단의 다음 부분을 목적지 주소로 변경 하시기 바랍니다.

````cpp
char dest_ip[] = "222.xxx.xxx.xxx";
int  dest_port = 50001;
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
WIZnet IoT shield + WIoT-QC01 인터페이스 보드와 ST Nucleo-L476RG 보드를 결합하여 PC와 USB로 연결합니다.
운영체제의 장치관리자를 통해 보드가 연결된 COM 포트를 확인 할 수 있습니다.

#### 4.2 Set up serial terminal and TCP server

보드와 통신을 위한 시리얼 터미널 프로그램과 TCP server 프로그램을 준비합니다.

> 예제에서는 시리얼 모니터링을 위해 잘 알려진 Tera term 프로그램과 TCP server로 Hercules 프로그램을 활용합니다.


Tera term 프로그램을 실행하여 이전 단계에서 확인된 보드의 COM 포트와 Baudrate 115200을 선택하여 시리얼 포트를 연결합니다.
이제 보드에서 출력하는 시리얼 메시지를 확인 할수 있습니다.

> 디버그 메시지 출력용 시리얼 포트 설정 정보: 115200-8-N-1, None

Hercules 프로그램의 TCP server 탭에서 포트 번호를 입력하고 `Listen` 버튼을 누르면 TCP server가 시작됩니다.

![][1]



#### 4.3 Send a message from device to teminal (Send)

성공적으로 연결이 완료된 후 보드를 리셋하면 다음과 같은 시리얼 출력을 확인 할 수 있습니다.
예제 코드는 Cat.M1 모듈의 상태를 확인한 후 TCP client로 동작하여 지정된 목적지로 `Hello Cat.M1` 메시지를 전송합니다.

TCP server에서 Cat.M1 모듈이 전송한 메시지를 확인할 수 있습니다.

![][2]

![][3]

#### 4.3 Send a message from terminal to device (Receive)

예제 코드는 TCP server 측에서 보내온 메시지가 Cat.M1 모듈로 정상 수신 되면 시리얼 디버그 포트를 통해 출력하도록 구성되어 있습니다.
보내진 `helloworld!` 메시지가 시리얼 포트로 출력되는 것을 확인 할 수 있습니다.

> 예제 코드는 `exit`를 수신하면 소켓 연결을 종료하고 프로그램을 마치도록 구현되어 있습니다.

![][4]

![][5]

<a name="ReadMore"></a>
## 더 보기
* [MBED 기반의 Cat.M1 HTTP 활용 가이드][mbed-guide-bg96-http]
* [MBED 기반의 Cat.M1 MQTT 활용 가이드][mbed-guide-bg96-mqtt]



[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-tcp-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_TCP(IP)_AT_Commands_Manual_V1.0.pdf

[mbed-getting-started]: ./mbed_get_started.md
[mbed-guide-bg96-http]: ./mbed_guide_bg96_http.md
[mbed-guide-bg96-mqtt]: ./mbed_guide_bg96_mqtt.md

[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_guide_bg96_tcp-1.png
[2]: ./imgs/mbed_guide_bg96_tcp-2.png
[3]: ./imgs/mbed_guide_bg96_tcp-3.png
[4]: ./imgs/mbed_guide_bg96_tcp-4.png
[5]: ./imgs/mbed_guide_bg96_tcp-5.png
