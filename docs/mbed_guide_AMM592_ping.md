# MBED 기반의 Cat.M1 Ping 테스트 가이드

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
| [ST Nucleo L476-RG][link-nucleo-l476rg] | WIoT-AM01 (AMM592) |

<a name="Step-1-Overview"></a>
## 소개
본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo L476-RG 보드를 이용하여 Cat.M1 단말의 Ping 테스트 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 Ping 테스트를 위한 통신 과정은 다음과 같은 순서로 구현합니다.

1. Echo 모드 설정
2. USIM 상태 확인
3. 네트워크 접속 확인
4. 네트워크 인터페이스(PDP Context) 활성화
5. Ping 테스트

<a name="Step-2-ATCommand"></a>
## AT 명령어

### 1. Echo 모드 설정

**AT Command:** ATE

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | ATE[n] | OK | ATE0<br>OK |

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| [n] | Integer | 0 : Echo mode OFF<br>1 : Echo mode ON |

### 2. USIM 상태 확인

**AT Command:** AT+CPIN

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | AT+CPIN? | +CPIN:[code]<br><br>OK | AT+CPIN?<br>+CPIN:READY<br><br>OK |

**Defined values:**


| Parameter | Type | Description |
|:--------|:--------|:--------|
| [code] | String | READY: PIN1 코드 조합이 끝난 상태<br> SIM PIN: PIN1 코드 조합 대기 <br> SIM PUK: PIN Lock 상태. PIN Lock 해제 코드 [PUK] %입력 대기 <br> |

### 3. 네트워크 접속 확인

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


### 5. Ping 테스트

**AT Command:** AT@PING6

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT@PING6="[IPv6 Addr] -c[count]" | OK | AT@PING6="2001:4860:4860::8888 -c 8"<br><br>OK<br>PING<br>2001:4860:4860::8888(2001:4860:4860::8888) 56 data bytes<br>64 bytes from 2001:4860:4860::8888: icmp_seq=1 ttl=49 time=391 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=2 ttl=49 time=102 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=3 ttl=49 time=123 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=4 ttl=49 time=99.9 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=5 ttl=49 time=143 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=6 ttl=49 time=80.8 ms<br>80<br>64 bytes from 2001:4860:4860::8888: icmp_seq=7 ttl=49 time=118 ms<br>64 bytes from 2001:4860:4860::8888: icmp_seq=8 ttl=49 time=76.0 ms<br><br><br>--- 2001:4860:4860::8888 ping statistics ---<br>8 packets transmitted, 8 received, 0% packet loss, time 7011ms<br>rtt min/avg/max/mdev = 76.066/142.028/391.598/96.569 ms |


**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| [IPv6 Addr] | String | Host IP address (URL, IPv6) |
| [count] | Integer | Ping 송신 횟수 |

ps. IPv4 (기존에 많이 사용되고 있는 IP 주소) 의 경우, IPv6로 주소를 변형하여야 합니다. (64:ff9b:: Header Use)
ex) IPv4 주소 123.123.123.123 의 경우 -> HEX로 변환 7B.7B.7B.7B
    ->64:ff9b::7b7b:7b7b

<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

```
/* AMM592 Ping Test */

// AT 명령어 echo 비활성화
ATE0

OK

// USIM 상태 확인 (READY면 정상)
AT+CPIN?
+CPIN:READY

OK

// 망 접속 확인 (+CEREG: 0,1 이면 접속)
AT+CEREG?
+CEREG: 0,1

OK

// PDP context 활성화
AT+CGATT=?
+CGATT: 1

OK

// Ping 테스트
AT@PING6="2001:4860:4860::8888 -c 4"
@PING6:ping6 2001:4860:4860::8888

OK
PING 2001:4860:4860::8888(2001:4860:4860::8888) 56 data bytes
64 bytes from 2001:4860:4860::8888: icmp_seq=1 ttl=49 time=329 ms
64 bytes from 2001:4860:4860::8888: icmp_seq=2 ttl=49 time=121 ms
64 bytes from 2001:4860:4860::8888: icmp_seq=3 ttl=49 time=137 ms
64 bytes from 2001:4860:4860::8888: icmp_seq=4 ttl=49 time=101 ms

--- 2001:4860:4860::8888 ping statistics ---
4 packets transmitted, 4 received, 0% packet loss, time 3005ms
rtt min/avg/max/mdev = 101.432/172.622/329.946/91.726 ms

```

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project
MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-AMM592-Ping | https://os.mbed.com/users/vikshin/code/WIZnet-IoTShield-AMM592-Ping |


> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.


### 2. Modify parameters

Ping 테스트 예제 코드는 별도로 수정할 Parameter가 없습니다.

### 프로그램 컴파일

상단 메뉴의 `Compile` 버튼을 클릭합니다.

![][compile]

![][10]

컴파일이 성공적으로 완료되면 `project.boardname.bin` 파일이 생성되어 download 됩니다.

> Chrome 브라우저의 경우, 하단 다운로드 바에서 생성된 바이너리 파일을 확인할 수 있습니다.

> 예제 파일이 성공적으로 컴파일 되면 `WIZnet-IoTShield-AMM592-Ping.NUCLEO_L476RG.bin`이 생성됩니다.

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
![][1] 

[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/


[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01


[serialMonitor]: ./imgs/arduino_guide_ide_serialmonitor.png

[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/mbed_guide_AM01_ping-01.PNG
[10]: ./imgs/mbed_get_started-10.png
[11]: ./imgs/mbed_get_started-11.png
