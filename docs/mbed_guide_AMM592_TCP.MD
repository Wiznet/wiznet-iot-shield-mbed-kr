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
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-AM01 (AM592) |

<a name="Step-1-Overview"></a>
## 소개
본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo-L476RG 보드를 이용하여 Cat.M1 단말의 TCP 데이터 송수신 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 TCP client(UDP 포함)의 통신 과정은 다음과 같은 순서로 구현합니다.

1. 네트워크 인터페이스 활성화
2. 소켓 열기 - 목적지 IP 주소 및 포트번호 포함
3. 데이터 전송 - 송신 및 수신
4. 소켓 닫기
5. 네트워크 인터페이스 비활성화

추가적으로, TCP 가이드 문서에는 다른 응용 가이드 문서에는 포함되어 있지 않은 Cat.M1 단말의 상태 확인 및 PDP context 관련 명령어에 대한 내용이 함께 포함되어 있습니다. 해당 명령어는 응용 구현 시 필수적으로 활용되어야 하므로, 함께 확인하시기 바랍니다.
* Echo 모드 설정: `ATE`
* USIM 상태 확인: `AT+CPIN?`
* 망 등록 및 상태 점검: `AT+CEREG`



<a name="Step-2-ATCommand"></a>
## AT 명령어

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
| Read | AT+CPIN? | +CPIN: &lt;code&gt; | AT+CPIN?<br>+CPIN: READY<br><br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| &lt;code&gt; | String | **READY**: PIN1 코드 조합이 끝난 상태<br>SIM PIN: PIN1 코드 조합 대기<br>SIM PUK: PIN Lock상태. PIN Lock해제 코드(PUK) ※입력 대기 |

### 3. 망 등록 및 상태 점검

망 서비스 상태 확인을 위해 사용되는 명령어 입니다. 디바이스 구현 시, 망 연결 유지를 위해 주기적으로 체크하는 것을 권장합니다.

**AT Command:** AT+CEREG

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT+CEREG=[n] | OK | AT+CEREG=0<br>OK |
| Read | AT+CEREG? | +CEREG: [n],[stat]<br>OK | AT+CEREG?<br>+CEREG:0,1<br>OK |
| Read | AT+CEREG=?| +CEREG: (list of supported [n]s) | AT+CEREG=?<br>+CEREG: (0-2,4)<br>OK|


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| [n] | Integer | 0 : 알림하지 않습니다. <br>	1 :	+CEREG[stat]의 자동알림을 유효하게 합니다. 네트워크 등록 상태가 변화했을 경우, 외부장치에 알립니다. <br>	2 :	+CEREG[stat][[[tac],[rac_mme],[ci],[act]]의 자동알림을 유효하게 합니다. 네트워크 등록 상태가 변했을 경우, 외부장치에 알립니다. <br> 4: +CEREG: [stat][,[[tac],[[ci],[[AcT][,,[,[[Active-Time]],[[Periodic TAU]]의 PSM을 사용하는 단말의 자동알림을 유효하게 합니다. 네트워크 등록상태가 변했을 경우, 외부장치에 알립니다. 
| [stat] | Integer | 0 : 등록 되지 않음(검색하지 않음)<br>1 : 등록됨<br>2 : 등록 되지 않음(검색 중)<br>3 : 등록 거부<br>4 : Unknown<br>5 : 등록됨(로밍)<br> |
| [tac] | Max 1024 char | Tracking Area Code 값으로 Hexadecimal 형식입니다. |
| [rac_or_mme] | Max 1024 char | RAC 값 혹은 MME 값을 표기합니다. Hexadecimal 형식입니다. |
| [ci] | Max 1024 char | Cell ID 값을 표기합니다. Hexadecimal 형식입니다. |
| [act] | Integer | Access Technology 값을 표기합니다. <br>0: GSM access technology<br>1: GSM Compact access technology<br>2:UMTS access technology<br> 3:EGPRS access technology<br> 4: HSDPA access technology<br> 5:HSUPA access technology <br> 6: HSDPA & HSUPA access technology <br> 7: LTE access technology <br> 8: LTE M1 (A/Gb) access technology<br> 9: LTE NB (S1) access technology <br> 10: Unspecified access technology |
| [Active-Time] | One byte in an 8 bit format | Active Time value를 표시합니다.<br> 3Bit: (단위) <br> -"000" 2초<br>-"001" 1분<br>-"010" 6분<br>5Bit: 시간 <br><br>예) "00100100" = 4분 |
|[Periodic-TAU]|One byte in an 8 bit format | Extended periodic TAU 를 표시합니다. <br>3Bit: (단위) <br> -"000" 10분 <br>-"001" 1시간 <br>-"010" 10시간 <br>-"011" 2초 <br>-"101" 1분 <br>-"110" 320 시간 <br>5Bit: 시간<br><br>예) "01000111" = 70 시간|

**AT Command:** AT@NSI

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT@NSI | @NSI: &lt;rssi/rsrp&gt;, &lt;srv_state&gt;, &lt;network_name&gt;, &lt;roam_state&gt;, &lt;RAT&gt;| AT@NSI<br>@NSI:4,"IN SRV","olleh","Home","LTE"<br><bR>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------:|:--------|
| &lt;rssi/rsrp&gt; | Integer| 신호의 세기 표시<br>[WCDMA] : RSSI 기준 <br> (rssi> 0) && (rssi<= : 4 <br> (rssi> 85 &&rssi<= : 3 <br>(rssi> 90 &&rssi<= : 2 <br>(rssi> 95 &&rssi<= : 1 <br> rssi < 100 : 0 <br> <bR> [LTE] : RSRP 기준 <br> (lte_rsrp>= 95) : 4 <br>(lte_rsrp>= 112) : 3 <br>(lte_rsrp>= 1 20 : 2 <br>(lte_rsrp>= 1 27 : 1 <br>(lte_rsrp>= 1 30 : 0|
| &lt;srv_state&gt; | 16 characters | 서비스상태 표시 <br>SRV NONE : 초기치입니다 <br>NO SRV : 서비스 불가능한 상태 <br>LIMITED : 긴급호등 제한된 서비스 가능상태 <br>IN SRV : 정상 서비스 가능 상태 <br>LIMITED REGIONAL : 제한 지역에서의 서비스 상태 <br>PWR SAVE : 전원 절약 상태<br> |
| &lt;network_name&gt; | 16 characters | Network name |
| &lt;roam_state&gt | 4 characters | 로밍상태를 표시 <br>Home : 홈네트워크·<br> Roam : 로밍네트워크|
| &lt;RAT&gt; | 5 characters | RAT(Radio Access Tcchnology) 를표시 <br> NONE: 초기치입니다.<br> GSM : 글로벌시스템의모바일통신(GSM) 모드 <br>GPRS : 일반패킷, 라디오서비스모드 <br>EGPRS : EGPRS 서포트모드 <br>UMTS : WideBandCDMA(WCDMA) 모드 <br>HSDPA : Wide BandCDMA(WCDMA) 모드 HSDPA 서포트 <br>HSUPA : Wide BandCDMA(WCDMA) 모드 HSUPA 서포트 <br>HSPA : Wide BandCDMA(WCDMA) 모드 HSDPA와 HSUPA 서포트 <br>LTE : Long Term Evolution 모드 |

### 4. PDP Context 활성화
> PDP(Packet Data Protocol)란 단말과 외부 패킷 데이터 네트워크 사이의 데이터 송수신을 위한 연결을 제공하기 위해 사용하는 네트워크 프로토콜을 뜻하며, PDP Context는 이러한 연결 과정에서 사용되는 정보의 집합을 의미합니다.

**AT Command:** AT+CGATT

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write  | AT+CGATT=[&lt;state&gt;] | OK | AT+CGATT=1<br>OK |
| Write  | AT+CGATT=, | +CGATT: &lt;state&gt;<br><br>OK | AT+CGATT=?<br>+CGATT: 1<br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| &lt;state&gt;| Integer | 0 : 접속 상태를 detach 설정<br>1 :	접속 상태를 attach 설정 |


### 5. 소켓 열기

소켓 서비스를 오픈하는 명령어 입니다.
> 네크워크가 IPv6 만 지원하는 경우 , IPv4 서버와 통신을 위해서는 IP v 4 서버 주소를 DNS 등록하여야 합니다.
등록 후 @DNSQ6 를 이용하여 DNS query 의 결과로 DNS64 서버로부터 AAAA ’ type 의 IPv6 주소를 획득할 수 있습니다 획득 된 주소를 이용하여 IPv6 소켓 통신 명령어를 이용할 수 있습니다.

**AT Command:** AT@SOCKOPEN6

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT@SOCKOPEN6=&lt;socket_id&gt;,&lt;socket_type&gt;,&lt;data_type&gt;,&lt;dest_ip&gt;,&lt;dest_port&gt;,&lt;local_port&gt; | OK <br><bR> @SOCKOPEN6:&lt;socket_id&gt;,&lt;result&gt; <br><bR> &lt;result&gt;: 범위 0-1 <br> 0: 성공 <br> 1: 실패 | AT@SOCKOPEN6=0,0,0,111.222.333.444,100<br>0<br>OK<br><bR>@SOCKOPEN6:0,0 |

TCP/UDP 소켓을 생성하고 , 서버로 접속을 합니다 . 최대 5 개의 소켓이 생성 가능합니다.

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| &lt;socket_id&gt; | Integer | Socket ID (범위: 0-4) |
| &lt;socket_type&gt; | Integer | Socket Type (범위: 0-1) <br> 0: TCP <br> 1: UDP |
| &lt;data_type&gt; | Integer | Data Type (범위: 0-1) <br> 0: ASCII mode <br> 1: HEXA mode |
| &lt;dest_ip&gt; | String | 접속 할 서버 IP (0000-ffff):(0000-ffff):(0000-ffff):(0000-ffff):(0000-ffff):(0000-ffff):(0000-ffff):(0000-ffff) |
| &lt;dest_port&gt; | Integer | 접속할 서버 PORT (범위: 0 - 65535)  |
| &lt;local_port&gt; | Integer | 데이터를수신할Port를지정합니다. <socket_type>이UDP에서만유효합니다. (범위: 0-65535) |


### 6. 소켓 종료

지정된 소켓 서비스를 종료하는 명령어 입니다.

**AT Command:** AT@SOCKCLOSE6

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT@SOCKCLOSE6=&lt;socket_id&gt; | OK <br><bR>@SOCKCLOSE6:&lt;socket_id&gt;,&lt;result&gt; <br><bR> &lt;result&gt;: 범위 0-1 <br> 0: 성공 <br> 1: 실패 | AT@SOCKCLOSE6=0<br>OK<br>@SOCKCLOSE6:0,0 |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| &lt;socket_id&gt; | Integer | Socket ID (범위: 0-4) |


### 7. 소켓 데이터 전송

지정된 소켓으로 데이터를 전송하는 명령어 입니다.

**AT Command:** AT@SOCKSEND6

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT@SOCKSND6=&lt;socket_id&gt;,&lt;data_type&gt;,&lt;data&gt; | OK<br>@SOCKSND6: &lt;socket_id&gt;,&lt;length&gt;,&lt;result&gt; | &lt;ASCII mode&gt; <br> AT@SOCKSND6=0,0,"1234" <br> OK<br> @SOCKSND6:0,4,0<br><br>&lt;HEXA mode&gt;<br>AT@SOCKSND6=0,1,"31323334"<br>OK<br>@SOCKSND6:0,4,0 |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| &lt;socket_id&gt; | Integer | 소켓 서비스 인덱스 (범위: 0-4) |
| &lt;data_type&gt; | Integer | Data Type (범위: 0-1) <br> 0: ASCII mode <br> 1: HEXA mode  |
| &lt;data&gt; | String | 전송할 데이터의 길이 (최대 1024 char)<br> 서버에 전송할 데이터로 전송 할 데이터를 " " 로 입력합니다|

**【주의】ASCII로 허용되지 않는 데이터를 전송해야하는 경우는 HEX 모드를 이용하시기 바랍니다.**
-ASCII mode인경우
&emsp;ASCII 문자가입력가능합니다. (모든ASCII 문자셋을지원하는것은아닙니다.)
&emsp;허용된ASCII문자: 0x20 ~ 0x7F에해당하는문자(0x2c, 0x3b 제외)
&emsp;금지된ASCII 문자: 0x00 ~ 0x1F에해당하는문자
-HEXA mode인경우
&emsp;Hexa값을문자로입력가능합니다.
&emsp;0에서9까지의숫자와A에서F까지의영문문자가입력가능합니다.
&emsp;0x00,0x01,0x02를서버에전송하고자할경우에는AT@SOCKWRITE=000102로입력합니다. 즉1개의데이터를전송하기위해서는2개의문자입력이필요합니다.



### 8. 소켓 데이터 수신

지정된 소켓의 버퍼로 데이터가 수신 될 경우, 모듈은 @SOCKRCV6=&lt;socket_id&gt;,&lt;data_type&gt;,&lt;length&gt;,&lt;data&gt; 명령을 출력합니다


| Type | Syntax | Example |
|:--------|:--------|:--------|
| Read | @SOCKRCV6=&lt;socket_id&gt;,&lt;data_type&gt;,&lt;length&gt;,&lt;data&gt;|  &lt;ASCII mode&gt; <br> @SOCKRCV6=0,0,4,ABCD <br><br>&lt;HEXA mode&gt; <br> @SOCKRCV6=0,1,4,61626364|


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| &lt;socket_id&gt; | Integer | 소켓 서비스 인덱스 (범위: 0-4) |
| &lt;data_type&gt; | Integer | Data Type (범위: 0-1) <br> 0: ASCII mode <br> 1: HEXA mode |
| &lt;length&gt; | Integer | 서버로부터 수신 받은 데이터 길이입니다 (범위: 0-1024) |
| &lt;data&gt; | String | 서버로부터 수신 받은 데이터 (범위: 0-1024 Char) <br> -ASCII mode 인 경우 <br>0x00 ~ 0x7F의 값에 해당 문자가 알림된다<br>-HEXA mode 인 경우<br>Hexa값을 2 개의 문자로 표기하여 알림된다|

### 9. 소켓 상태 확인

현재 소켓 상태를 확인하는 명령입니다.


| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write |AT@SOCKSTAT6| @SOCKSTAT6:&lt;socket_id&gt;,&lt;socket_type&gt;,&lt;socket_status&gt;| AT@SOCKSTAT6<br>@SOCKSTAT6:0,0,0<br>@SOCKSTAT6:1,1,3<br>@SOCKSTAT6:2,0,2<br>@SOCKSTAT6:3,0,0<br>@SOCKSTAT6:4,0,0<br><br>OK


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| &lt;socket_id&gt; | Integer | 소켓 서비스 인덱스 (범위: 0-4) |
| &lt;socket_type&gt; | Integer | Data Type (범위: 0-1) <br> 0: TCP <br> 1: UDP |
| &lt;socket_status&gt; | Integer | 범위: 0-3 <br> 0 : Socket Closed <br> 1 : Socket Closing<br>2 : Socket Opening <br>3 : Socket Opened |

### 10. IP 주소 획득

서버의 도메인 이름으로 서버의 IP 주소를 획득합니다.


| Type | Syntax | Example |
|:--------|:--------|:--------|
| Write | AT@DNSQ6=&lt;domain_name&gt;|  AT@DNSQ6=www.google.com <br> OK<br><br> @DNSQ6:2404:6800:4004:80a::2003|


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| &lt;domain_name&gt; | String | Max 255 |
| &lt;ip_address&gt; | Integer | (0000 ffff):(0000 ffff):(0000ffff):(0000 ffff):(0000 ffff):(0000ffff):(0000 ffff):(0000 ffff) |


<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

```
/* AM01 TCP Send/Recv */

// AT 명령어 echo 비활성화
ATE0

// USIM 상태 확인 (READY면 정상)
AT+CPIN?

// 망 접속 확인 (SRV면 접속)
AT+CEREG
AT@NSI

// APN과 IP 프로토콜 확인 (IPv6 only)
AT@DNSQ6=www.naver.com

// TCP socket open (목적지 IP주소 및 포트번호)
AT@SOCKOPEN6=0,0,0,64:ff9b::xxxx:xxxx,8000

// TCP Send
AT@SOCKSND6=0,0,"1234"

// TCP Receive
@SOCKRCV6=0,0,4,ABCD

// TCP socket close
AT@SOCKCLOSE=0

// TCP socket close 여부 확인
AT@SOCKSTAT

```

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project

MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-BG96-TCP | https://os.mbed.com/users/vikshin/code/WIZnet-IoTShield-AMM592-TCP/ |


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

상단 메뉴의 `Compile` 버튼을 클릭합니다.

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
WIZnet IoT shield + WIoT-AM01 인터페이스 보드와 ST Nucleo-L476RG 보드를 결합하여 PC와 USB로 연결합니다.
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




[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-tcp-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_TCP(IP)_AT_Commands_Manual_V1.0.pdf

[mbed-getting-started]: ./mbed_get_started.md
[mbed-guide-bg96-http]: ./mbed_guide_bg96_http.md


[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_guide_bg96_tcp-1.png
[2]: ./imgs/mbed-guide_AM01_tcp-02.PNG
[3]: ./imgs/mbed-guide_AM01_tcp-03.PNG
[4]: ./imgs/mbed-guide_AM01_tcp-04.PNG
[5]: ./imgs/mbed-guide_AM01_tcp-05.PNG
