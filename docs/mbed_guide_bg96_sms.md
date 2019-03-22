# MBED 기반의 Cat.M1 SMS 데이터 통신 가이드

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [AT 명령어](#Step-2-ATCommand)
-   [동작 구조 예제](#Step-3-SampleCode)
-   [예제 코드 빌드 및 실행](#Step-4-Build-and-Run)


<a name="Prerequisites"></a>
## 시작하기 전에

> * 하드웨어 설정과 개발환경 구축은 **[MBED 기반으로 Cat.M1 디바이스 개발 시작하기][mbed-getting-started]** 문서에 상세히 설명되어 있습니다.

> * Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]**를 기준으로 작성되었습니다.

> * 아래 AT Command에 대한 설명은 SMS 기능 구현에 꼭 필요한 명령어만 설명하고 있습니다. 보다 자세한 설명은 Cat.M1 모듈 매뉴얼을 참고하시기 바랍니다.


### Development Environment
* **[Arm MBED Web Compiler][link-mbed-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-(WM-N400MSE)<br>WIoT-QC01 (BG96) |

<a name="#Step-1-Overview"></a>
## 소개
본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo-L476RG 보드를 이용하여 Cat.M1 단말의 SMS 데이터 송수신 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, TCP/IP 소켓 통신에 비해서 비교적 간단한 AT Command로 테스트를 할 수 있습니다.
이번 테스트에서는 일반적인 스마트폰을 이용하여, Cat M1 모듈의 SMS 송수신 예제를 설명하도록 하겠습니다.


<a name="#Step-2-ATCommand"></a>
## AT 명령어

> 좀 더 상세한 AT 명령어 설명은 Quectel과 우리넷의 AT Command Manual에서 확인 하실 수 있습니다.
> * [BG96_AT_Commands_Manual_V2.1][link-bg96-atcommand-manual]
> * WM-N400MSE_AT_Commands_Manual

### 1. Cat M1 모듈의 일반적인 동작 설명은 생략

>  Cat M1 모듈의 에코 모드 설명, USIM 상태 확인, 네트워크 접속 확인 등의 일반적인 내용은 TCP 가이드를 참고하시기 바랍니다.

### 2. 우리넷 SMS 명령어 
우리넷 모듈에서 제공하는 SMS 명령이 많이 있지만, 본 장에서는 테스트에 사용된 명령어에 대해서만 설명합니다. 자세한 내용은 우리넷 매뉴얼을 참고하시기 바랍니다.

**AT Command:** AT*SMSMO 문자 메시지 송신

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Write | AT*SMSMO=(value1),(value2),(value3),(value4),(value5) | OK or ERROR | AT&#42;SMSMO=010xxxxxxxx,010yyyyyyyy,1,1,BEC8B3E7C7CFBCBCBFE43F<br>OK<br>*SMSACK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (value1) | Message Index |
| (value2) | 0 : 읽지 않은 메시지로 남김<br>1 : 읽은 메시지로 변경 |
| (value3) | 수신 시간 ( 포맷 : YYYYMMDDHHMMDD ) |
| (value4) | Callback Number ( 회신 번호 ) |
| (value5) | Data ( 포맷 : Hex ) |

**AT Command:** AT&#42;SKT&#42;READMT 문자 메시지 수신

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Read | AT&#42;SKT&#42;READMT=(value1) | &#42;SKT&#42;READMT:(value1),(value2),(value3),(value4),”(value5)” | AT&#42;SKT&#42;READMT=1<br>&#42;SKT&#42;READMT:1,20130109114519,01040144365,4098,TEST00000<br>OK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (value1) | 수신 메시지 Index<br>Index 는 가장 최근에 수신된 메시지부터 ‘0’ 값을 가진다. |
| (value2) | 수신 시간 ( 포맷 : YYYYMMDDHHMMDD ) |
| (value3) | Callback Number ( 회신 번호 ) |
| (value4) | Tele-service ID |
| (value5) | 수신된 문자 메시지 내용 |

참고로, Cat M1 모듈에서 한글 메시지는 EUC-KR로 인코딩 되어 처리됩니다.

아래는 이번 문서의 테스트에 사용된 한글 문자열을 EUC-KR Hex 코드로 변환한 내용들입니다.
>  오픈하우스 : BF C0 C7 C2 C7 CF BF EC BD BA
>  안녕하세요? : BE C8 B3 E7 C7 CF BC BC BF E4 3F


**[ Note ]**
EUC-KR 한글 변환은 온라인 상에서 손쉽게 확인해보실 수 있습니다.
> * https://dencode.com/string

**AT Command:** AT&#42;SKT&#42;NEWMSG 문자 메시지 즉시 알림

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Write | AT&#42;SKT&#42;NEWMSG=(value1) | &#42;SKT&#42;NEWMSG:(value1)<br>&#42;SKT&#42;NEWMSG:(value2),(value3),(value4),(value1),(value5) | AT&#42;SKT&#42;NEWMSG?<br>&#42;SKT&#42;NEWMSG:4098<br>OK<br>&#42;SKT&#42;NEWMSG:4,20130109124620,0104014xxxx,4098,New Message |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (value1) | Tele-service ID |
| (value2) | 수신 메시지 Index |
| (value3) | 수신 시간 ( 포맷 : YYYYMMDDHHMMDD ) |
| (value4) | Callback Number ( 회신 번호 ) |
| (value5) | 수신된 문자 메시지 내용 |


### 3. 앰투앰넷 SMS 명령어
앰투앰넷 모듈에서는 SMS 송수신 방식으로 PDU 모드와 Text 모드를 지원하고 있습니다.
Text 모드에서는 영문과 특수 기호만 지원되며, 한글은 발신만 지원됩니다.

> PDU 모드 활용을 위한 PDU 인코딩/디코딩 알고리즘은 앰투앰넷 측에 요청하시기 바랍니다.
 
본 문서에서는 Text 모드에서 간단한 영문 SMS 송수신을 테스트 합니다. 자세한 내용은 앰투앰넷 매뉴얼을 참고하시기 바랍니다.

**AT Command:** AT+CSCS SMS String 방식 설정

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Test | AT+CSCS=? | +CSCS: (list of supported chsets)<br> OK| - |
| Read | AT+CSCS? | +CSCS: (chset) OK| - |
| Write | AT+CSCS=(chset) | OK |AT+CSCS="IRA"<br>OK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (chset) | GSM(GSM default alphabet)<br>IRA(International reference alphabet)<br>UCS2(UCS2 alphabet) |


**AT Command:** AT+CMGF SMS 메시지 포맷

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Test | AT+CMGF=? | +CMGF: (list of supported modes)<br> OK| - |
| Read | AT+CMGF? | +CMGF: (mode) OK| - |
| Write | AT+CMGF=(mode) | OK |AT+CMGF=1<br>OK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (mode) | 0 PDU mode<br>1 Text mode |


**AT Command:** AT+CMGS SMS 메시지 전송

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Test | AT+CMGS=? | OK| - |
| Write | AT+CMGS=(da) | OK | AT+CMGS="1502101xxxx"<br>> This is a test from .... // 보낼 SMS  데이터 입력 후 Ctrl + z<br>+CMGS: 247<br>OK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (da) | SMS를 보낼 수신 전화번호 |


**AT Command:** AT+CMGR SMS 메시지 수신

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Test | AT+CMGR=? | OK| - |
| Write | AT+CMGR=(index)|+CMGR: (stat),(oa),(alpha),(scts),(tooa),(fo),(pid),(dcs),(sca),(tosca),(length)<br>(data)<br>OK | AT+CMGR=3<br>+CMGR: "REC UNREAD","0107576xxxx",,"19/02/26,09:17:13+36"<br>00480069<br>OK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (index) | 수신된 SMS 인덱스 |
**[ Note ]**
AT Command의 자세한 파라미터는 해당 모듈의 매뉴얼을 참고하시기 바랍니다.

<a name="#Step-3-SampleCode"></a>

## 동작 구조 예제 (우리넷 Cat.M1 SMS)

```
AT*SMSMO=0107576****,,1,1,48656C6C6F            // “Hello” 전송
OK

*SMSACK

AT*SMSMO=0107576****,,1,1,BFC0C7C2C7CFBFECBDBA  // “오픈하우스” 전송
OK

*SMSACK

AT*SKT*READMT=0                                  // “Hi” 수신
*SKT*READMT:0,20190225165426,0107576****,4098,Hi

OK

AT*SKT*READMT=0                                  // “안녕하세요?” 수신
*SKT*READMT:0,20190225165505,0107576****,4098,안녕하세요?

OK
```
**[ Note ]**
EUC-KR이 적용되지 않은 PC 일부 터미널에서는 한글이 깨져 보일 수 있으니 주의 하시기 바랍니다.
> AT*SKT*READMT=0
> *SKT*READMT:0,20190225165505,0107576****,4098,¾È³çÇÏ¼¼¿ä?
>
> OK

스마트폰에서 SMS 로그는 아래와 같습니다.
![][1]


## 동작 구조 예제 (앰투앰넷 Cat.M1 SMS)

```
AT+CMGF=1                          // Message 포맷을 Text mode로 
OK
AT+CSCS="GSM"                      // 캐릭터셋을 GSM으로 
OK
AT+CMGS="0107576****"              // Message를 보낼 전화 번호 
> Hello                            // 보낼 Message, 입력 완료는 Ctrl + Z
> 
+CMGS: 5

OK

+CMTI: "ME",5
AT+CMGR=5
+CMGR: "REC UNREAD","0107576****",,"19/02/26,12:59:39+36"
00480069

OK
```

스마트폰에서 SMS 로그는 아래와 같습니다.
![][2]


<a name="#Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project

MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-BG96-SMS | https://os.mbed.com/users/hkjung/code/WIZnet-IoTShield-BG96-SMS/ |


> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.

![][import1]

링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.

![][import2]

### 2. Modify parameters

SMS 발송을 위한 목적지 전화번호와 내용을 변경하여 테스트 해 볼 수 있습니다.

````cpp
char phone_number[] = "010xxxxxxxx";
char send_message[] = "WIZnet Cat.M1 IoT shield is powered on";
````

### 3. Compile

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 SMS 발신 및 수신 기능의 구현을 확인 하실 수 있습니다.

> * 예제 샘플 코드는 보드가 부팅되면 사용자의 스마트폰으로 메시지를 발송하고, 사용자 스마트폰에서 보낸 메지시를 시리얼 터미널에 출력하도록 구성되어 있습니다.

#### 4.1 Functions

```cpp
int8_t initSMS_BG96(void);
```
 * SMS 기능 사용을 위한 설정을 수행합니다.
 * SMS 메시지 포맷(PDU 모드 / Text 모드)과 Chracter set(GSM / IRA / USC2) 을 설정합니다. 예제는 text 모드에 GSM으로 설정되어 있습니다.

```cpp
int8_t sendSMS_BG96(char * da, char * msg, int len);
```
 * SMS 메시지를 발송합니다. DA(destination address)는 전화번호, 메시지와 메시지 길이를 파라메터로 전달합니다.

```cpp
int checkRecvSMS_BG96(void);
```
  * SMS 메시지의 수신 여부를 체크합니다. 만약 메시지가 수신되면 수신 메시지 index를 리턴합니다.
  * 메인 루프 안에서 동작해야 합니다.

```cpp
int8_t recvSMS_BG96(int msg_idx, char * msg, char * da, char * datetime);
```
 * 지정한 메시지 index의 메시지를 사용자 버퍼로 받아옵니다. 발신자의 DA(전화번호), 날짜시간, 메시지를 파라메터를 통해 얻게 됩니다.

```cpp
int8_t deleteSMS_BG96(int msg_idx);
```
 * 지정한 index의 메시지를 삭제합니다.

```cpp
int8_t deleteAllSMS_BG96(int delflag);
```
 * delflag에 따라 지정 카테고리의 모든 메시지를 삭제합니다.
   * *1: Delete all read messages from storage*
   * *2: Delete all read messages from storage and sent mobile originated messages*
   * *3: Delete all read messages from storage, sent and unsent mobile originated messages*
   * *4: Delete all messages from storage*

#### 4.2 Demo

![][3]

![][4]


[mbed-getting-started]: ./mbed_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-http-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_HTTP(S)_AT_Commands_Manual_V1.0.pdf


[1]: ./imgs/mbed_guide_bg96_sms-1.png
[2]: ./imgs/mbed_guide_bg96_sms-2.png
[3]: ./imgs/mbed_guide_bg96_sms-3.png
[4]: ./imgs/mbed_guide_bg96_sms-4.png


[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
