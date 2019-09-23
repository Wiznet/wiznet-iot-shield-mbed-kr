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
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-AM01 (AMM592) |

<a name="Step-1-Overview"></a>
## 소개
본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo-L476RG 보드를 이용하여 Cat.M1 단말의 HTTP 데이터 송수신 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, 일반적인 HTTP Client의 통신 과정은 다음과 같은 순서로 구현합니다.

1. 네트워크 인터페이스 활성화
2. HTTP 설정 - 목적지 URL, 옵션 
3. HTTP Request 전송
4. HTTP Response 확인
5. 네트워크 인터페이스 비활성화

<a name="Step-2-ATCommand"></a>
## AT 명령어



### 1. Cat M1 모듈의 일반적인 동작 설명은 생략

>  Cat M1 모듈의 에코 모드 설멍, USIM 상태 확인, 네트워크 접속 확인, PDP Context 활성화 등의 일반적인 내용은 TCP 가이드를 참고하시기 바랍니다.

### 2. HTTP Data 전송

AT@HTTPSND 명령은 HTTP 서버에게 GET Request를 전송합니다. 이 명령을 입력하고, 일정 시간 동안 HTTP Response를 기다려야 합니다.

**AT Command:** AT@HTTPSND

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT@HTTPSND=? | @HTTPSND:(0-2),(0-10),,,<br><br>OK | -|
| Write | AT@HTTPSND=&lt;method&gt;,&lt;ssl_enable&gt;,&lt;hex_mode&gt;,&lt;url&gt;,&lt;header&gt;,&lt;body&gt; | OK <br><br>@HTTPSND:&lt;result&gt;| GET request (ASCII) <br> AT@HTTPSND=0,0,0,http:// hosturl/path,”-H ‘header string’”,”Body string” <br><br> GET request (HEX)<br>AT@HTTPSND=0,0,1,http:// hosturl/path,”2D48202C”,”426F64”<br><br>OK<br><br>@HTTPSND:0 <br><br> POST request <br> AT@HTTPSND=1,0,0,http:// hosturl/path, -H‘header string’,Body string <br>OK<br><br>@HTTPSND:0<br>HEAD request<br>AT@HTTPSND=2,0,0,http:// hosturl/path<br>OK<br><br> @HTTPSND:0 |


**Defined values:**

| Parameter | 범위 | Description |
|:--------|:--------|:--------|
| &lt;method&gt; | (0-2) | HTTP Request method를 지정합니다.<br> 0: GET<br>1: POST<br>:2: HEAD  |
| &lt;ssl_enable&gt; | (0-10) | SSL 보안 기능의 사용 여부를 지정합니다. SSL 보안 기능을 사용하지 않는 경우(0) 이외에는 인증서의 index를 사용합니다. <br> 0: SSL Disable <br> 1-10: File index for certificate|
| &lt;hex_mode&gt; | (0-1) | HTTP send 및 receive 시 header 와 body data format 을 ASCII 또는 HEX data 로 사용 여부를 선택합니다. <br> 0: ASCII 입력 모드 <br> 1: HEX입력 모드 |
| &lt;url&gt; | Max 512 char | HTTP Server 의 http:// 혹은 https:// 를 포함하는 URL 정보입니다. |
| &lt;header&gt; | Max 512 char | HTTP Header 정보를 입력합니다. 여러 개의 Header 를 추가할 수 있으며 “ “로 표시합니다. 각 Header 의 구분은 –H 를 시작으로 ‘ ‘로표시합니다. <br> e.g) “–H ‘Connection=keep-alive’ –H ‘Content-Type=multipart/form-data’” <br>ASCII 또는 HEX 로 입력 가능하며 ASCII 의 경우 허용된 ASCII 문자는 0x20 ~ 0x7F 에 해당하는 문자입니다. |
| &lt;body&gt; | Max 512 char | HTTP Body 정보를 입력합니다. " " 로표시합니다. <br> HEX 또는 ASCII 로 입력 가능하며 ASCII 의 경우 허용된 ASCII 문자는 0x20 ~ 0x7F 에 해당하는 문자입니다. |
| &lt;result&gt; | (0-1) | 0: 성공 <br> 1: 실패 |

### 3. HTTP Response 확인

AT@HTTPRCV 명령은 모듈이 소켓데이터를수신할 경우알림입니다. 

**AT Command:** AT@QHTTRCV

**Syntax:**

| Syntax |  Example |
|:--------|:--------|
| AT@HTTPRCV=&lt;length&gt;, &lt;data&gt; | (ASCII mode) <br> @HTTPRCV=15,HTTP/1.1 200 <br>(HEX mode) <br> @HTTPRCV:15,486565502F3120323030|


**Defined values:**

| Parameter | 범위| Description |
|:--------|:--------|:--------|
| &lt;length&gt; | (1-1024) | 서버로 부터 수신 받은 데이터 길이입니다  |
| &lt;data&gt; | Max 1024 char | 서버로 부터 수신 받은 데이터 |


<a name="Step-3-SampleCode"></a>

## 동작 구조 예제 #1 (기상청 웹서버에 접속하여 분당구 날씨 확인)

```
// Send HTTP Get Request. 
AT@HTTPSND=0,0,0, http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=4113552000
OK

// Content-Length:1024 수신 
@HTTPRCV:1024, <?xml version="1.0" encoding="UTF-8" ?>
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
| WIZnet-IoTShield-BG96-HTTP | https://os.mbed.com/users/vikshin/code/WIZnet-IoTShield-AMM592-HTTP/ |


> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.

![][import1]

링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.

![][import2]

### 2. Modify parameters

만약 다른 서비스의 API 테스트를 위해 URL을 변경하려는 경우, 다음 변수의 내용을 변경하면 됩니다.

````cpp
// Sample HTTP URL: Weather info by Korea Meteorological Administration
char request_url[] = "http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=4113552000";
````

### 3. Compile

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 HTTP client 운용 방법에 대해 파악할 수 있습니다.



#### 4.1 Functions
 
```cpp
int8_t sendHttpRequest_AMM592(int method, int ssl_enable, int hex_mode, char * url);
```
 * setHttpRequest 함수에 설정한 URL로 HTTP Request를 전송합니다. 

```cpp
int8_t getHttpResponse_BG96(void);
```
  * 모듈 버퍼로 수신된 HTTP response를 읽어옵니다. 





#### 4.2 Demo

HTTP 요청에 대한 응답으로 기상청 날씨 정보를 획득하여 시리얼 터미널을 통해 출력합니다.

![][1]
<br>.<br>
![][2]

[mbed-getting-started]: ./mbed_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-http-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_HTTP(S)_AT_Commands_Manual_V1.0.pdf

[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png

[1]: ./imgs/mbed_guide_AM01_http-01.PNG
[2]: ./imgs/mbed_guide_AM01_http-02.PNG
