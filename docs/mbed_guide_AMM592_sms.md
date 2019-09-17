# MBED 기반의 Cat.M1 SMS 활용하기

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

> * 아래 AT Command에 대한 설명은 SMS 기능 구현에 꼭 필요한 명령어만 설명하고 있습니다. 보다 자세한 설명은 Cat.M1 모듈 매뉴얼을 참고하시기 바랍니다.


### Development Environment
* **[Arm MBED Web Compiler][link-mbed-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------|
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-AM01(AM592) |

<a name="Step-1-Overview"></a>
## 소개
본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo-L476RG 보드를 이용하여 Cat.M1 단말의 SMS 데이터 송수신 방법에 대한 가이드를 제공합니다.

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. Cat.M1 모듈 제조사에 따라 AT 명령어의 차이는 있지만, TCP/IP 소켓 통신에 비해서 비교적 간단한 AT Command로 테스트를 할 수 있습니다.
이번 테스트에서는 일반적인 스마트폰을 이용하여, Cat M1 모듈의 SMS 송수신 예제를 설명하도록 하겠습니다.


<a name="Step-2-ATCommand"></a>
## AT 명령어



### 1. Cat M1 모듈의 일반적인 동작 설명은 생략

>  Cat M1 모듈의 에코 모드 설명, USIM 상태 확인, 네트워크 접속 확인 등의 일반적인 내용은 TCP 가이드를 참고하시기 바랍니다.

### 2. AM Telecom SMS 명령어
AM Telecom 모듈에서는 SMS 송수신 방식으로 PDU 모드와 Text 모드를 지원하고 있습니다.
Text 모드에서는 영문과 특수 기호만 지원되며, 한글은 발신만 지원됩니다.

본 문서에서는 Text 모드에서 간단한 영문 SMS 송수신을 테스트 합니다. 자세한 내용은 AM Telecom 매뉴얼을 참고하시기 바랍니다.

**AT Command:** AT+CSCS SMS String 방식 설정

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+CSCS=? | +CSCS: (파라미터범위)<br> OK| AT+CSCS=? <br>+CSCS:("GSM","UCS2")<br>OK |
| Read | AT+CSCS? | +CSCS: &lt;chset&gt; OK| AT+CSCS?<br>+CSCS: "GSM"<br>OK |
| Write | AT+CSCS=[&lt;chset&gt;] | OK |AT+CSCS="GSM"<br>OK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (chset) | GSM(GSM7bit Default Alphabet)<br>UCS2(Universal Multi-octet Character Set2) |


**AT Command:** AT+CMGF SMS 메시지 포맷

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+CMGF=? | +CMGF: (0-1)<br> OK| AT+CMGF=? <br>+CMGF:(0-1)<br>OK |
| Read | AT+CMGF? | +CMGF: (mode) OK| AT+CMGF <br>+CMGF: 0<br>OK |
| Write | AT+CMGF= [&lt;mode&gt;] | OK |AT+CMGF=1<br>OK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| [mode]| 0: PDU mode<br>1: Text mode |

**AT Command:** AT+CNMI 수신 SMS 표시 설정 명령어

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+CNMI=? | +CNMI:(0-2),(0-3),(0-2),(0-2),(0,1)<br> OK| AT+CNMI=?<br>+CNMI: (0,1,2),(0,1,2,3),(0,2),(0,1,2),(0,1)<br>OK |
| Read | AT+CNMI? | +CNMI: &lt;mode&gt;,&lt;mt&gt;,&lt;bm&gt;,&lt;ds&gt;,&lt;bfr&gt; <br> OK| AT+CNMI? <br>+CNMI: 2,1,0,0,0<br>OK |
| Write | AT+CNMI=[&lt;mode&gt;],[&lt;mt&gt;],[&lt;bm&gt;],[&lt;ds&gt;],[&lt;bfr&gt;]| OK |AT+CNMI=2,1,0,0,0<br>OK|

**Defined values:**

| Parameter | 범위 | Description |
|:--------|:--------|:--------|
| &lt;mode&gt; | (0-2) |수신한 SMS 의 알림 방법을 설정합니다. <br> 0: SMS 알림과 SMS의 전송 알림을 모듈의SMS 알림용 버퍼에 저장하고, 외부장치에는 전송하지 않습니다. 모듈의SMS 알림용 버퍼가 가득차게 되면, 가장 오래된 알림을 새로운 알림으로 덮어씁니다.(초기값) <br> 1 : SMS 알림과 SMS의 전송알림을 직접 외부장치에 전송합니다. 전송할 수 없을때는 알림을 파기합니다. <br> 2 : SMS 알림과 SMS의 전송 알림을 직접 외부장치에 전송합니다. 전송할 수 없을때는 SMS 알림을 모듈의 SMS 알림용 버퍼에 저장하고 전송 가능한때에 외부장치에 전송합니다. |
| &lt;mt&gt;| (0-3) | 수신한 SMS의 저장 방법이나 알림 방법을 설정합니다. <br> 0 : SMS의 수신을 외부장치에 알림지 않습니다. <br> 1 : 수신한 SMS를 모듈 또는 USIM카드에 저장하고, 저장위치를 외부장치로 전송합니다. <br> 2 : 수신한 SMS를 모듈 또는 USIM카드에 저장하지 않고 직접외부장치에 전송합니다.<br> 3 : Class3 SMS의 경우는, 직접전송(+CMT) 합니다(mt가 2인 경우와같다). Class3 이외의 SMS인 경우는, 모듈내에 저장하고 저장위치를 전송(+CMTI)합니다 (mt가 1인 경우와같다). |
| &lt;bm&gt;| (0,2)| 0 : Cell Broadcast Messages 를 DTE에 전송하지 않습니다. <br> 2 : Cell Broadcast Messages를 unsolicited result code 와함께 DTE에 전송합니다. |
| &lt;ds&gt;|(0-2)| SMS의 전송 알림에대해 설정합니다. <br> 0 : SMS의 전송알림을 모듈에 저장하지 않고, 외부장치에도 전송하지 않습니다. <br> 1 : SMS의 전송 알림을 모듈에 저장하지 않고 직접 외부장치에 전송하고, unsolicited reault code(+CDS)로 전송합니다. <br> &emsp; &lt;mode&gt;를0 또는 2로 설정하는경우, 본파라미터를 1로 설정하면 안됩니다.|
| &lt;bfr&gt;| (0,1)| 0 : <mode>=1..2 가입력 되었을 때 , unsolicited result code 의 TA buffer 를 TE 에전송됩니다. <br>1 : <mode>=1..2 가입력되 었 을때 , unsolicited result code 의 TA buffer 를 초기화합니다. |


**AT Command:** AT+CMGS SMS 메시지 전송

**Syntax:**

| Type | Syntax | Response | Example |
|:--------|:--------|:--------|:--------|
| Test | AT+CMGS=? | OK| - |
| Write | AT+CMGS=&lt;da&gt;, [&lt;toda&gt;]| +CMGS: &lt;mr&gt;,[&lt;scts&gt;] | AT+CMGS="1502101xxxx", 129<br>> This is a test from .... // 보낼 SMS  데이터 입력 후 Ctrl + z<br>+CMGS: 247<br>OK |

**Defined values:**

| Parameter | 범위 | Description |
|:--------|:--------|
| &lt;da&gt; | (0-20) | SMS를 보낼 수신 전화번호 |
| &lt;toda&gt; | (129,145) | SMS 수신측단말의 전화번호 타입<br>129 : National numbering scheme <br>145 : International numbering scheme("+" 포함) |

>파라미터를 생략 했을 경우의 동작 &lt;toda&gt;가 설정되지 않은경우 , &lt;da&gt; 의첫번째값이 「+」인 경우 &lt;toda&gt; 는 「145」 입니다. 「+」 이외의 값 일때 &lt;toda&gt; 는 「129」가됩니다.

**AT Command:** AT+CMGR SMS 의 Index로 SMS 읽기명령

**Syntax:**

| Type | Syntax | Response | Example
|:--------|:--------|:--------|:--------|
| Test | AT+CMGR=? | OK| - |
| Write | AT+CMGR=&lt;index&gt;|+CMGR: <br> &lt;stat&gt;,&lt;oa&gt;,&lt;alpha&gt;,&lt;scts&gt;[,&lt;tooa&gt;,&lt;fo&gt;,&lt;pid&gt;,&lt;dcs&gt;,&lt;sca&gt;,&lt;tosca&gt;,&lt;length&gt;] <br> OK | AT+CMGR=3<br>+CMGR: "REC UNREAD","0107576xxxx",,"19/02/26,09:17:13+36"<br>SMS Test Message<br>OK |

**Defined values:**

| Parameter | 범위|  Description |
|:--------|:--------|:--------|
| &lt;index&gt;| (0-24) | 수신된 SMS 인덱스 |



<a name="Step-3-SampleCode"></a>


## 동작 구조 예제 (AM Telecom Cat.M1 SMS)

```
AT+CMGF=1                          // Message 포맷을 Text mode로 
OK
AT+CSCS="GSM"                      // 캐릭터셋을 GSM으로 
OK
AT+CNMI=2,1,0,0,0                  // 수신 SMS 의 표시 방법 설정 
OK
AT+CMGS="0107*******"              // Message를 보낼 전화 번호 
> Hello World                            // 보낼 Message, 입력 완료는 Ctrl + Z
> 
+CMGS: 12

OK
@NOTI:45,메시지 발신 요청 완료

+CMTI: "ME",0
AT+CMGR=0
+CMGR: "REC UNREAD","0107*******",,"19/09/10,10:03:52+36"
Hello Cat.M1

OK
```

스마트폰에서 SMS 로그는 아래와 같습니다.
![][2]


<a name="Step-4-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project

MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-BG96-SMS | https://os.mbed.com/users/vikshin/code/WIZnet-IoTShield-AMM592-SMS/ |


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
int8_t initSMS_AMM592(void);
```
 * SMS 기능 사용을 위한 설정을 수행합니다.
 * SMS 메시지 포맷(PDU 모드 / Text 모드)과 Chracter set(GSM / IRA / USC2) 을 설정합니다. 예제는 text 모드에 GSM으로 설정되어 있습니다.

```cpp
int8_t sendSMS_AMM592(char * da, char * msg, int len);
```
 * SMS 메시지를 발송합니다. DA(destination address)는 전화번호, 메시지와 메시지 길이를 파라메터로 전달합니다.

```cpp
int checkRecvSMS_AMM592(void);
```
  * SMS 메시지의 수신 여부를 체크합니다. 만약 메시지가 수신되면 수신 메시지 index를 리턴합니다.
  * 메인 루프 안에서 동작해야 합니다.

```cpp
int8_t recvSMS_AMM592(int msg_idx, char * msg, char * da, char * datetime);
```
 * 지정한 메시지 index의 메시지를 사용자 버퍼로 받아옵니다. 발신자의 DA(전화번호), 날짜시간, 메시지를 파라메터를 통해 얻게 됩니다.

```cpp
int8_t deleteSMS_AMM592(int msg_idx);
```
 * 지정한 index의 메시지를 삭제합니다.

```cpp
int8_t deleteAllSMS_AMM592(int delflag);
```
 * delflag에 따라 지정 카테고리의 모든 메시지를 삭제합니다.
   * *0: &lt;index&gt;로지정된 SMS 를삭제합니다.*
   * *1: 메모리에서 모든 읽은 SMS를 삭제합니다. 읽지않은 SMS와 발신 SMS를 제외합니다.*
   * *2: 메모리에서 모든 읽은 SMS와 발신 SMS를 삭제합니다. 읽지않은 SMS와 전송되지않은 SMS를 제외합니다*
   * *3: 메모리에서 모든 읽은 SMS와 발신 SMS를 삭제합니다. 읽지않은 SMS를 제외합니다.*
   * *4: 메모리에 있는 모든 SMS를 삭제합니다.*


#### 4.2 Demo

![][3]

![][4]


[mbed-getting-started]: ./mbed_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-http-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_HTTP(S)_AT_Commands_Manual_V1.0.pdf

[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png

[2]: ./imgs/mbed-guide_AM01_sms-2.png
[3]: ./imgs/mbed-guide_AM01_sms-3.png
[4]: ./imgs/mbed-guide_AM01_sms-4.png


[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
