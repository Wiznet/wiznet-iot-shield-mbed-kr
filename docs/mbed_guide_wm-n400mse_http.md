# MBED 기반의 Cat.M1 HTTP 데이터 통신 가이드

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

> * 아래 AT Command에 대한 설명은 HTTP 연동에 꼭 필요한 명령어만 설명하고 있습니다. 보다 자세한 설명은 Cat M1 모듈 매뉴얼을 참고하시기 바랍니다.

### Development Environment
* **[Arm MBED Web Compiler][link-mbed-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-WM01 (WM-N400MSE) |

<a name="Step-1-Overview"></a>
## 소개
본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo-L476RG 보드를 이용하여 Cat.M1 단말의 HTTP 데이터 송수신 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 HTTP Client의 통신 과정은 다음과 같은 순서로 구현합니다.

1. 네트워크 인터페이스 활성화
2. HTTP 설정 - 목적지 URL, 옵션 
3. HTTP Request 전송
4. HTTP Response 확인
5. 네트워크 인터페이스 비활성화

우리넷 Cat.M1 모듈의 경우에는 현재 HTTP 관련 AT 명령어를 지원하지 않아 TCP 관련 AT 명령어를 이용하여 HTTP 메소드를 사용하는 방법으로 작성되었습니다.

<a name="Step-2-ATCommand"></a>
## AT 명령어

> 좀 더 상세한 AT 명령어 설명은 우리넷의 AT Command Manual에서 확인 하실 수 있습니다.
> * [WM-N400MSE_AT_Commands_Guide_v1.1][link-wm-n400mse-atcommand-manual]

### 1. Cat M1 모듈의 일반적인 동작 설명은 생략

>  Cat M1 모듈의 에코 모드 설명, USIM 상태 확인, 네트워크 접속 확인, PDP Context 활성화 등의 일반적인 내용은 TCP 가이드를 참고하시기 바랍니다.

### 2. 소켓 생성
소켓 서비스를 생성하는 명령어 입니다.

**AT Command:** AT+WSOCR

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+WSOCR=(value1),(value2),(value3),(value4),(value5) | +WSOCR:(value6),(value7),(value8),(value9)<br><br>OK | AT+WSOCR=0,www.kma.go.kr,80,1,0<br>+WSOCR:1,0,64:ff9b::8b96:f9a2/80,TCP<br><br>OK |

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

### 3. 소켓 연결
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

### 4. 소켓 데이터 전송

지정된 소켓으로 데이터를 전송하는 명령어 입니다.

**AT Command:** AT+WSOWR

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Write | AT+WSOWR=(value1),(value2),(value3) | +WSOWR:(value4),(value5)<br><br>OK | AT+WSOWR=0,93,GET /wid/queryDFSRSS.jsp?zone=4113552000 HTTP/1.1<br>HOST: www.kma.go.kr<br>Connection: close<br>(enter)<br><br>+WSOWR:1,0<br><br>OK |

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

### 5. 소켓 데이터 수신
지정된 소켓으로부터 데이터를 수신하는 명령어 입니다.

**AT Command:** +WSORD

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Read | | +WSORD=(value1),(value2),(value3),(value4) OK<br>(value5) | +WSORD:0,1024,HTTP/1.1 200 OK<br>Date: Wed, 18 Sep 2019 00:45:41 GMT<br>Content-Length: 9174<br>Accept-Ranges: bytes<br>Content-Type: text/xml; charset=UTF-8<br>Connection: close<br><br><?xml version="1.0" encoding="UTF-8" ?><br>...

**Defined values:**

| Parameter | Type | Description |
|:--------|:--------|:--------|
| (value1) | integer | Socket ID |
| (value2) | integer | Buffer Size |
| (value3) | string | HTTP Version |
| (value4) | integer | Status Code |
| (value5) | string | Data |

### 6. 소켓 종료
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
## 동작 구조 예제 (기상청 웹서버에 접속하여 분당구 날씨 확인)
```
// 소켓 생성
AT+WSOCR=0,www.kma.go.kr,80,1,0
+WSOCR:1,0,64:ff9b::8b96:f9a2/80,TCP

OK
// 소켓 연결
AT+WSOCO=0
+WSOCO:1,0,OPEN_WAIT

OK
+WSOCO:0,OPEN_CMPL
// 소켓 데이터 전송
AT+WSOWR=0,93,GET /wid/queryDFSRSS.jsp?zone=4113552000 HTTP/1.1
HOST: www.kma.go.kr
Connection: close


+WSOWR:1,0

OK
// 소켓 데이터 수신 (분당구 수내동의 기상 정보 확인)
+WSORD:0,1024,HTTP/1.1 200 OK
Date: Wed, 18 Sep 2019 00:45:41 GMT
Content-Length: 9174
Accept-Ranges: bytes
Content-Type: text/xml; charset=UTF-8
Connection: close

<?xml version="1.0" encoding="UTF-8" ?>
<rss version="2.0">
<channel>
<title>기상청 동네예보 웹서비스 - 경기도 성남시분당구 수내1동 도표예보</title>
<link>http://www.kma.go.kr/weather/main.jsp</link>
<description>동네예보 웹서비스</description>
<language>ko</language>
.....................................................................
<wfKor>맑음</wfKor>
.....................................................................
```

<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project

MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-WM-N400MSE-HTTP | https://os.mbed.com/users/AustinKim/code/WIZnet-IoTShield-WM-N400MSE-HTTP/ |


> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.

![][import1]

링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.

![][import2]

### 2. Modify parameters

만약 다른 서비스의 API 테스트를 위해 URL을 변경하려는 경우, 다음 변수의 내용을 변경하면 됩니다.

````cpp
// Destination (Remote Host)
// Domain Name and Port number
char dest_domain_name[] = "www.kma.go.kr";
int  dest_port = 80;
````
````cpp
// REST(Representational State Transfer) API
char rest_data_1st[] = "GET /wid/queryDFSRSS.jsp?zone=4113552000 HTTP/1.1";
char rest_data_2nd[] = "HOST: www.kma.go.kr";
char rest_data_3rd[] = "Connection: close";
````

### 3. Compile

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 HTTP client 운용 방법에 대해 파악할 수 있습니다.

> * 예제 샘플 코드는 `동작 구조 예제` 를 코드로 구현한 것입니다. 만약 구체적인 응용 프로젝트로 구현될 경우, 수신된 HTTP response 메시지를 parsing하여 원하는 정보를 획득 후 이에 해당하는 동작 구현이 추가되는 것이 일반적입니다.

#### 4.1 Functions

```cpp
int8_t sockOpenConnect_WM01(int protocol, const char *addr, int port, int pckttype);
```
 * client로 동작하며 소켓 서비스를 생성 및 연결을 합니다.
 * server 접속 정보(domain name, port number), 전송을 진행 할 message packet type을 설정합니다.

```cpp
int8_t sockClose_WM01(void);
```
 * 지정된 소켓 서비스의 연결을 종료합니다.

```cpp
int8_t sendData_WM01(void);
```
  * 지정된 소켓으로 데이터를 전송합니다.

```cpp
int8_t recvData_WM01(void);
```
 * 지정된 소켓으로부터 데이터를 수신합니다.



#### 4.2 Demo

HTTP 요청에 대한 응답으로 기상청 날씨 정보를 획득하여 시리얼 터미널을 통해 출력합니다.

![][1]



[mbed-getting-started]: ./mbed_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-wm-n400mse-atcommand-manual]: ./datasheet/WM-N400MSE_AT_Commands_Guide_v1.1.pdf

[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png

[1]: ./imgs/mbed_guide_wm-n400mse_http-1.png
