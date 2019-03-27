# MBED 기반의 Cat.M1 AWS 연동 가이드

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

> * MQTT 등 공통 AT Command에 대한 세부적인 설명은 생략하였습니다. 자세한 설명은 Cat M1 모듈 매뉴얼을 참고하시기 바랍니다.
>   * [MBED 기반의 Cat.M1 MQTT 데이터 통신 가이드][mbed-guide-bg96-mqtt]


### Development Environment
* **[Arm MBED Web Compiler][link-mbed-compiler]**

### Hardware Requirement

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-QC01 (BG96) |

<a name="Step-1-Overview"></a>
## 소개
AWS IoT란 무엇입니까?

> AWS IoT은(는) 인터넷 연결 제품(센서, 액추에이터, 내장형 마이크로 컨트롤러, 스마트 애플리케이션 등)과 AWS 클라우드 간에 안전한 양방향 통신을 제공합니다. 이를 통해 여러 디바이스에서 원격 측정 데이터를 수집하고 해당 데이터를 저장 및 분석할 수 있습니다. 또한 사용자가 휴대전화 또는 태블릿에서 이러한 디바이스를 제어할 수 있게 해주는 애플리케이션을 만들 수도 있습니다.
> 출처 : [AWS IoT란 무엇입니까?](https://docs.aws.amazon.com/ko_kr/iot/latest/developerguide/what-is-aws-iot.html)

![][0]

본 문서에서는 Cat M1 모듈을 사용하여, AWS IoT 서비스에 접속하는 방법에 대한 가이드를 제공합니다.

먼저, AWS IoT 서비스 상에서 사물을 생성하고, 그에 수반되는 인증서를 생성하고, 보안 정책을 설정하는 등 AWS 서비스 상에서 선행되는 사전 작업에 대한 간단한 가이드를 설명하고 있습니다. 그리고, Cat M1 모듈에서 AT Command를 통하여, AWS IoT에 접속하고 데이터를 주고 받는 간단한 예제를 설명하고 있습니다.

AWS IoT에 대한 자세한 설명은 AWS IoT 개발자 안내서를 참고하시기 바랍니다.

> AWS 개발자 안내서: [AWS IoT 시작하기](https://docs.aws.amazon.com/ko_kr/iot/latest/developerguide/iot-gs.html)


이 가이드 문서는 아래와 같은 과정으로 구성되어 있습니다.

1. AWS - 사물 생성하기
2. AWS - 인증서 및 보안 정책 생성
3. AWS - 엔드 포인트 및 섀도우 확인
4. 디바이스 - 인증서 파일 저장하기
5. 디바이스 - AWS IoT에 접속하기
6. 디바이스 - 데이터 게시
7. 디바이스 - 데이터 구독
8. 디바이스 - AWS IoT 접속 해제하기
9. 디바이스 - AT 명령어 전체 로그

<a name="Step-2-ATCommand"></a>
## AT 명령어

> 좀 더 상세한 AT 명령어 설명은 Quectel AT Command Manual에서 확인 하실 수 있습니다.
> * [BG96_AT_Commands_Manual_V2.1][link-bg96-atcommand-manual]
> * [BG96_MQTT_Application_Note_V1.0][link-bg96-mqtt-an]
> * [BG96_SSL_AT_Commands_Manual_V1.0][link-bg96-ssl-manual]
> * [BG96_FILE_AT_Commands__V1.0][link-bg96-file-manual]

### 1. 파일 삭제

Cat.M1 모듈 파일시스템의 파일을 삭제할 때 사용되는 명령어 입니다. 인증서를 업데이트 하기 위해 사용됩니다.

**AT Command:** AT+QFDEL

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QFDEL=? | +QFDEL: (filename) <br>OK| - |
| Write | AT+QFDEL=(filename) | OK <br> +CME ERROR: (err)|AT+QFDEL=*<br>OK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (filename) | Name of the file to be deleted. The maximum length of <filename> is 80 bytes.<br>"*" Delete all the files. |

### 2. 파일 업로드

Cat.M1 모듈 파일시스템에 파일을 업로드 할 때 사용되는 명령어 입니다. 인증서를 업데이트 하기 위해 사용됩니다.

**AT Command:** AT+QFUPL

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QFUPL=? | +QFUPL: (filename)[,(1-(freesize0)[,(1-65535)[,(0,1)]]] <br>OK| - |
| Write | AT+QFUPL=(filename)[,(file_size)[,(timeout)[,(ackmode)]]] | CONNECT<br>+QFUPL: (upload_size),(checksum)<br>OK <br> +CME ERROR: (err)|AT+QFUPL="AmazonRootCA1.pem",1188,100<br>CONNECT<br>+QFUPL: 1188,2d13<br><br>OK |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
| (freesize) | The free space size of (name_pattern). |
| (filename) | Name of the file to be uploaded. The maximum length is 80 bytes. |
| (file_size) | The file size expected to be uploaded. |
| (upload_size) | The actual size of the uploaded data.. |
| (timeout) | The time waiting for data to be inputted to USB/UART. |
| (ackmode) | Whether to use ACK mode. |
| (checksum) | The checksum of the uploaded data. |


### 3. SSL/TLS Context 파라메터 설정

**AT Command:** AT+QSSLCFG

**Syntax:**

| Type | Syntax | Respones | Example
|:--------|:--------|:--------|:--------|
| Test | AT+QSSLCFG=? | +QSSLCFG: "sslversion",(0-5),(0-4)<br>+QSSLCFG:<br>"ciphersuite",(0-5),(0X0035,0X002F,0X0005,0X0004,0X000A,0X003D,0XC011,0XC012,0XC013,0XC014,0XC027,0XC028,0XC02F,0XFFFF)<br>+QSSLCFG: "cacert",(0-5),(cacertpath)<br>+QSSLCFG: "clientcert",(0-5),(client_cert_path)<br>+QSSLCFG: "clientkey",(0-5),(client_key_path)<br>+QSSLCFG: "seclevel",(0-5),(0-2)<br>+QSSLCFG: "ignorelocaltime",(0-5),(0,1)<br>+QSSLCFG: "negotiatetime",(0-5),(10-300) <br>OK| - |
| Write |AT+QSSLCFG="sslversion",(sslctxID)[,(sslversion)]        |+QSSLCFG: "sslversion",(sslctxID),(sslversion)       |AT+QSSLCFG="sslversion",2,4<br>OK                                      |
| Write |AT+QSSLCFG="ciphersuite",(sslctxID)[,(ciphersuites)]     |+QSSLCFG: "ciphersuite",(sslctxID),(ciphersuites)    |AT+QSSLCFG="ciphersuite",1,0x0035<br>OK                                |
| Write |AT+QSSLCFG="cacert",(sslctxID)[,(cacertpath)]            |+QSSLCFG: "cacert",(sslctxID),(cacertpath)           |AT+QSSLCFG="cacert",2,"AmazonRootCA1.pem"<br>OK                        |
| Write |AT+QSSLCFG="clientcert",(sslctxID)[,(client_cert_path)]  |+QSSLCFG: "clientcert",(sslctxID),(client_cert_path) |AT+QSSLCFG="clientcert",2,"815c124058-certificate.pem.crt"<br>OK       |
| Write |AT+QSSLCFG="clientkey",(sslctxID)[,(client_key_path)]    |+QSSLCFG: "clientkey",(sslctxID),(client_key_path)   |AT+QSSLCFG="clientkey",2,"815c124058-private.pem.key"<br>OK            |
| Write |AT+QSSLCFG="seclevel",(sslctxID)[,(seclevel)]            |+QSSLCFG: "seclevel",(sslctxID),(seclevel)           |AT+QSSLCFG="seclevel",2,2<br>OK                                        |
| Write |AT+QSSLCFG="ignorelocaltime",(sslctxID)[,(ignoreltime)]  |+QSSLCFG: "ignorelocaltime",(sslctxID),(ignoreltime) |AT+QSSLCFG="ignorelocaltime",1<br>+QSSLCFG: "ignorelocaltime",1,1<br>OK|
| Write |AT+QSSLCFG="negotiatetime",(sslctxID)[,(negotiate_time)] |+QSSLCFG: "negotiatetime",(sslctxID),(negotiate_time)|-                                                                      |

**Defined values:**

| Parameter |  Description |
|:--------|:--------|
|(sslctxID) |Numeric type. SSL context ID. |
|(sslversion) |Numeric type. SSL Version. <br>0 SSL3.0 <br>1 TLS1.0 <br>2 TLS1.1 <br>3 TLS1.2 <br>4 All |
|(ciphersuites) |Numeric type. SSL cipher suites. <br>0X0035 TLS_RSA_WITH_AES_256_CBC_SHA <br>0X002F TLS_RSA_WITH_AES_128_CBC_SHA <br>0X0005 TLS_RSA_WITH_RC4_128_SHA <br>0X0004 TLS_RSA_WITH_RC4_128_MD5 <br>0X000A TLS_RSA_WITH_3DES_EDE_CBC_SHA <br>0X003D TLS_RSA_WITH_AES_256_CBC_SHA256 <br>0XC011 TLS_ECDHE_RSA_WITH_RC4_128_SHA <br>0XC012 TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA <br>0XC013 TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA <br>0XC014 TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA <br>0XC027 TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256 <br>0XC028 TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384 <br>0XC02F TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 <br>0XFFFF Support all |
|(ignoreltime) |Numeric format. How to deal with expired certificate. <br>0 Care about validity check for certification <br>1 Ignore validity check for certification |
|(cacertpath) |String format. The path of the trusted CA certificate. |
|(client_cert_path) |String format. The path of the client certificate.| 
|(client_key_path) |String format, the path of the client private key. |
|(seclevel) |Numeric format. The authentication mode. <br>0 No authentication <br>1 Manage server authentication <br>2 Manage server and client authentication if requested by the remote server |
|(negotiate_time) |Numeric format. Indicates maximum timeout used in SSL negotiation stage. The value rang is 10-300, and the default value is 300.|

MQTT에 대한 AT 명령어 사용 설명은 아래 링크에서 확인 하실 수 있습니다.
> [MBED 기반의 Cat.M1 MQTT 데이터 통신 가이드][mbed-guide-bg96-mqtt]

<a name="Step-3-SampleCode"></a>
## 동작 구조 예제

### 1. AWS - 사물 생성하기
AWS IoT 계정에 등록된 디바이스는 레지스트리를 통해 AWS IoT 계정에 등록됩니다.
만약 AWS IoT를 처음 사용하는 사용자라면, 아래와 같은 화면을 보실 수 있는데, 여기에서 [사물 등록]을 시작하시면 됩니다.

![][1]

그리고, AWS IoT 사물 생성 페이지에서 [단일 사물 생성]을 선택합니다.

![][2]

사물 생성 페이지의 이름 필드에 디바이스 이름을 입력합니다. 이번 테스트에서는 [WIZnet_IoTShield_CatM1]라는 이름으로 디바이스를 추가하도록 하겠습니다.

![][3]

### 2. AWS - 인증서 및 보안 정책 생성

AWS IoT는 디바이스와의 통신에서 X.509 인증서를 사용합니다.
AWS IoT는 사용자를 위해 인증서를 생성할 수도 있고 사용자가 자체 X.509 인증서를 사용할 수도 있습니다. 
이번 가이드에서는 AWS IoT가 생성하는 인증서를 사용하도록 하겠습니다. 인증서는 사용 전에 활성화해야 합니다.

아래 화면에서는 사물에 인증서를 추가하는 과정입니다. [인증서 생성]을 선택하면 사물 인증에 사용하는 인증서, 퍼블릭 키, 프라이빗 키가 생성됩니다.

![][4]

이제, 인증서가 생성되었습니다.

아래 화면에서, 사물에 대한 인증서, 프라이빗 키, 루트 CA를 PC에 다운로드하시기 바랍니다.
여기에서 다운로드 한 파일들은 이후 Cat M1 모듈이 AWS IoT 서버에 접속될 때, 사용될 예정입니다.

![][5]

AWS IoT 정책은 디바이스가 MQTT 주제 구독 또는 게시와 같은 AWS IoT 작업을 수행할 수 있는 권한을 부여하는 데 사용합니다. 사용자의 디바이스는 AWS IoT로 메시지를 전송할 때 인증서를 제출합니다. 디바이스가 AWS IoT 작업을 수행하도록 허용하려면 AWS IoT 정책을 생성하여 디바이스 인증서에 연결해야 합니다.

먼저, 아래 그림에서 [정책 선택]을 선택하시기 바랍니다.

![][6]

정책을 생성할 때, 정책 이름에는 "WIZnet_IoTShield_CatM1_Policy"를 사용하겠습니다. 정책 이름에 개인 식별 정보를 사용하지 않는 것이 좋습니다.

[작업] 필드와 [리소스 ARN] 필드에 아래와 같이 입력합니다. 이제 모든 클라이언트가 AWS IoT에 연결할 수 있습니다.

> * 작업: iot:&#42;
> * 리소스 ARN: &#42;
> * 효과: 허용


만약 클라이언트 ARN을 리소스로 지정하면 연결할 수 있는 디바이스를 제한할 수 있습니다.

![][7]

이제, [보안] [인증서] 메뉴에서 인증서를 선택하여, 인증서를 [활성화]하도록 하겠습니다. 그리고, 인증서에 [정책 연결]을 선택합니다.

![][8]

아래 [인증서에 정책 연결] 화면에서는 전 단계에서 생성한 정책 "WIZnet_IoTShield_CatM1_Policy"를 선택합니다.

![][9]

### 3. AWS - 엔드 포인트 및 섀도우 확인

AWS 클라우드에서 디바이스의 영구적 표현을 위해서, 디바이스 섀도우 서비스를 하고 있습니다.
사용자는 디바이스 섀도우에 업데이트된 상태 정보를 게시할 수 있고, 디바이스 연결 시 상태를 동기화할 수 있습니다.
또한 디바이스는 애플리케이션 또는 다른 디바이스가 사용하도록 섀도우에 현재 상태를 게시할 수 있습니다.

아직 디바이스가 데이터 게시를 하지 않았기 때문에, 섀도우에 상태가 없습니다.

![][10]

[상호 작용]을 선택하면, Cat M1 모듈이 AWS IoT에 접속할 HTTPS 엔드포인트와 MQTT Topic을 확인할 수 있습니다.

![][11]

### 4. 디바이스 - 인증서 파일 저장하기

이제부터 Cat M1 모듈을 AWS IoT에 연결하는 과정을 설명합니다.
먼저, 이전 단계([2. AWS - 인증서 및 보안 정책 생성])에서 다운로드 받은 인증서 파일들을 Cat M1 모듈에 저장하는 과정입니다.

```
// Cat M1 모듈의 모든 파일 리스트 지우기
AT+QFDEL="*"

OK

// Cat M1 모듈의 파일 리스트 확인하기
AT+QFLST

OK

// 루트 CA 인증서를 Cat M1 모듈 파일에 업로드
AT+QFUPL="AmazonRootCA1.pem",1188,100
CONNECT
+QFUPL: 1188,2d13

OK

// 사물에 대한 인증서를 Cat M1 모듈 파일에 업로드
AT+QFUPL="815c124058-certificate.pem.crt",1220,100
CONNECT
+QFUPL: 1220,7317

OK

// 프라이빗 키를 Cat M1 모듈 파일에 업로드
AT+QFUPL="815c124058-private.pem.key",1675,100
CONNECT
+QFUPL: 1675,372

OK
```

### 5. 디바이스 - AWS IoT에 접속하기

이제, 디바이스가 AWS IoT에 접속하는 과정을 설명하도록 하겠습니다.

이전 단계(4. 디바이스 - 인증서 파일 저장하기)에서 Cat M1 모듈에 저장된 인증서 파일을 SSL 접속에 등록시키고, 여러가지 SSL 옵션을 설정합니다. 그리고, 엔드포인트 주소로 접속하여, AWS IoT에 접속합니다.
자세한 AT Command 명령어에 대한 설명은 해당 가이드 문서를 확인하시거나 매뉴얼을 확인하시기 바랍니다.

```
// SSL : 루트 CA 인증서 경로 설정
AT+QSSLCFG="cacert",2,"AmazonRootCA1.pem"

OK

// SSL : 클라이언트 인증서 경로 설정
AT+QSSLCFG="clientcert",2,"815c124058-certificate.pem.crt"

OK

// SSL : 클라이언트 프라이빗 키  경로 설정
AT+QSSLCFG="clientkey",2,"815c124058-private.pem.key"

OK

// Manage server and client authentication if requested by the remote server
AT+QSSLCFG="seclevel",2,2

OK

// TLS 1.1
AT+QSSLCFG="sslversion",2,4

OK

// Ciphersuite : TLS_RSA_WITH_AES_256_CBC_SHA
AT+QSSLCFG="ciphersuite",1,0x0035

OK

// Ignore validity check for certification
AT+QSSLCFG="ignorelocaltime",1
+QSSLCFG: "ignorelocaltime",1,1

OK

// MQTT 접속을 SSL로 설정
AT+QMTCFG="SSL",0,1,2

OK

// 엔드 포인트 주소 설정
AT+QMTOPEN=0,"a3uz5t2azg1xdz-ats.iot.ap-northeast-2.amazonaws.com",8883

OK

+QMTOPEN: 0,0

// AWS IoT에 접속
AT+QMTCONN=0,"WIZnet_IoTShield_CatM1"

OK

+QMTCONN: 0,0,0
```

### 6. 디바이스 - 데이터 게시

이제, 디바이스에서 AWS IoT로 데이터를 보내보도록 하겠습니다.

디바이스 섀도우 서비스는 디바이스가 섀도우 상태 정보를 가져오거나 업데이트하거나 삭제할 수 있도록 예약된 MQTT 토픽를 사용합니다.
이러한 토픽의 이름은 $aws/things/thingName/shadow로 시작합니다. 디바이스 섀도우를 업데이트하려면 $aws/things/thingName/shadow/update 토픽으로 상태 문서를 게시합니다. 상태 문서는 JSON 문서 포맷으로 전송이 됩니다.

사물의 상태를 업데이트하려는 클라이언트는 다음과 같은 JSON 요청 상태 문서를 전송합니다.

```
{
    "state" : {
        "desired" : {
            "temp" : "20"
            "led" : "on",
         }
     }
}
```

섀도우를 업데이트하는 디바이스는 다음과 같은 JSON 요청 상태 문서를 전송합니다.

```
{
    "state" : {
        "reported" : {
            "temp" : "25"
            "led" : "off",
         }
     }
}
```

이제, 디바이스(Cat M1 모듈)가 AWS IoT에게 데이터를 보내보도록 하겠습니다.

```
// AWS IoT에 데이터 Publish하기
AT+QMTPUB=0,1,1,0,"$aws/things/WIZnet_IoTShield_CatM1/shadow/update"
> {"state":{"reported":{"Temp":"29","Color":"Red"}}}

OK

+QMTPUB: 0,1,0
```

AWS IoT에서 해당 사물의 섀도우를 선택하면, 디바이스가 보낸 상태가 바로 적용되는 것을 확인하실 수 있습니다.

![][13]

### 7. 디바이스 - 데이터 구독

AWS IoT에서 디바이스에게 데이터를 전송하는 과정을 테스트 해보도록 하겠습니다.
먼저, 이번에는 디바이스가 $aws/things/WIZnet_IoTShield_CatM1/shadow/update/accepted를 구독하도록 하겠습니다.

```
// AWS IoT로부터 데이터 Subscribe하기
AT+QMTSUB=0,1,"$aws/things/WIZnet_IoTShield_CatM1/shadow/update/accepted",1

OK

+QMTSUB: 0,1,0,1
```

아래는 사물의 섀도우에서 "Color"를 "White"로 업데이트해 보도록 하겠습니다. 그럼, AWS IoT는 사물에게 업데이트된 정보를 전달하게 됩니다. 이를 위해서, 아래 화면에서 처럼, [섀도우], [편집]을 선택하여, 섀도우 상태를 아래와 같이 편집하고 [저장]을 선택합니다.

![][14]

이 때, AWS IoT는 사물에게 이 데이터를 전달하게 되고, 디바이스에서는 아래와 같은 메시지를 받게 됩니다.
```
+QMTSUB: 0,1,0,1

+QMTRECV: 0,1,"$aws/things/WIZnet_IoTShield_CatM1/shadow/update/accepted","{"state":{"reported":{"Temp":"29","Color":"Red"},"desired":{"Color":"White"}},"metadata":{"reported":{"Temp":{"timestamp":1553325144},"Color":{"timestamp":1553325144}},"desired":{"Color":{"timestamp":1553325144}}},"version":5,"timestamp":1553325144}"
```

이 데이터를 가독성 있게 정렬하면 아래와 같습니다. 즉, AWS IoT 섀도우 화면에서 수정한 내용이 그대로 적용되는 것을 확인하실 수 있습니다.

```
{ 
  "state":{
    "reported":{
      "Temp":"29",
      "Color":"Red"
    },
    "desired":{
      "Color":"White"
    }
  },
  .......................
```

### 8. 디바이스 - AWS IoT 접속 해제하기

만약 AWS IoT와 연결을 해제하려고 한다면, AT+QMTDISC 명령어를 사용하면 됩니다.

```
AT+QMTDISC=0
OK

+QMTDISC: 0,0
```

### 9. 디바이스 - AT 명령어 전체 로그

아래 그림은 이번 AWS IoT 테스트에 사용된 디바이스의 전체 로그입니다.

```
AT+QFDEL="*"

OK

AT+QFLST

OK

AT+QFUPL="AmazonRootCA1.pem",1188,100
CONNECT
+QFUPL: 1188,2d13

OK

AT+QFUPL="815c124058-certificate.pem.crt",1220,100
CONNECT
+QFUPL: 1220,7317

OK

AT+QFUPL="815c124058-private.pem.key",1675,100
CONNECT
+QFUPL: 1675,372

OK

AT+QSSLCFG="cacert",2,"AmazonRootCA1.pem"

OK

AT+QSSLCFG="clientcert",2,"815c124058-certificate.pem.crt"

OK

AT+QSSLCFG="clientkey",2,"815c124058-private.pem.key"

OK

AT+QSSLCFG="seclevel",2,2

OK

AT+QSSLCFG="sslversion",2,4

OK

AT+QSSLCFG="ciphersuite",1,0x0035

OK

AT+QSSLCFG="ignorelocaltime",1
+QSSLCFG: "ignorelocaltime",1,1

OK

AT+QMTCFG="SSL",0,1,2

OK

AT+QMTOPEN=0,"a3uz5t2azg1xdz-ats.iot.ap-northeast-2.amazonaws.com",8883

OK

+QMTOPEN: 0,0

AT+QMTCONN=0,"WIZnet_IoTShield_CatM1"

OK

+QMTCONN: 0,0,0

AT+QMTPUB=0,1,1,0,"$aws/things/WIZnet_IoTShield_CatM1/shadow/update"
> {"state":{"reported":{"Temp":"29","Color":"Red"}}}

OK

+QMTPUB: 0,1,0

AT+QMTSUB=0,1,"$aws/things/WIZnet_IoTShield_CatM1/shadow/update/accepted",1

OK

+QMTSUB: 0,1,0,1

+QMTRECV: 0,1,"$aws/things/WIZnet_IoTShield_CatM1/shadow/update/accepted","{"state":{"reported":{"Temp":"29","Color":"Red"},"desired":{"Color":"White"}},"metadata":{"reported":{"Temp":{"timestamp":1553325144},"Color":{"timestamp":1553325144}},"desired":{"Color":{"timestamp":1553325144}}},"version":5,"timestamp":1553325144}"

AT+QMTDISC=0

OK

+QMTDISC: 0,0

```


<a name="Step-4-Build-and-Run"></a>

## 예제 코드 빌드 및 실행

### 1. Import project

MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-BG96-Cloud-AWS | https://os.mbed.com/users/hkjung/code/WIZnet-IoTShield-BG96-Cloud-AWS/ |


> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.

![][import1]

링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.

![][import2]

### 2. Modify parameters
예제를 수행하기 위해서 AWS 인증서와 Endpoint, Thing 등이 수정되어야 합니다.
awscerts.h 파일과 aws_iot_config.h 파일의 다음 항목을 AWS IoT의 사물 정보와 일치하도록 수정합니다.

> RootCA, Client certificate, privatekey 등의 이름은 BG96 모듈의 파일 시스템에 저장하기 위한 용도이므로 그대로 사용하셔도 좋습니다.

````cpp
/* aws_iot_config.h */
#define AWS_IOT_MQTT_HOST              (char*)"a3uz5t2xxxxxxx-ats.iot.ap-northeast-2.amazonaws.com"
#define AWS_IOT_MQTT_PORT              8883
#define AWS_IOT_MQTT_CLIENT_ID         (char*)"Your_Thing_Name"
#define AWS_IOT_MY_THING_NAME          (char*)"Your_Thing_Name"
#define AWS_IOT_ROOT_CA_FILENAME       (char*)"AmazonRootCA1.pem"
#define AWS_IOT_CERTIFICATE_FILENAME   (char*)"certificate.pem.crt"
#define AWS_IOT_PRIVATE_KEY_FILENAME   (char*)"private.pem.key"
````

awscerts.h 의 인증서들은 `2. AWS - 인증서 및 보안 정책 생성` 단계에서 생성된 파일들의 내용을 입력합니다.
줄바꿈도 예제와 동일하게 입력하셔야 합니다.

````cpp
/* awscerts.h */
const char aws_iot_certificate[] = \
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDWTCCAkGgAwIBAgIUIW0yog+2PbY6m8dSCHFMbGDAV0cwDQYJKoZIhvcNAQEL\n"
    "BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
    "SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE5MDMyMzA2NDA0\n"
    ...
    "-----END CERTIFICATE-----\n";

const char aws_iot_private_key[] = \
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEogIBAAKCAQEAv35YqgykEtF3VtMfaaug3f1qacwGyda00/P8Ff35LLto3Ylq\n"
    "gE2dLCf039/Wpdxpgw/ONX27p7Z5EIb5gPFcMTb+vibZAFlLR+Due8RSGCJxUqV9\n"
    "jAfEvgEnUwdbJD6Sj77itsSkPfKp/p9lqBL0lNhl5vlTPKDxzxsrlOEB2MpFXcz6\n"
    ...
    "-----END RSA PRIVATE KEY-----\n";

const char aws_iot_rootCA[] = \
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
	...
    "-----END CERTIFICATE-----\n";
````

이 외에도 예제의 Publish 메시지 전송 주기, Publish 할 JSON 메시지 등을 자유롭게 변경 하실 수 있습니다.

### 3. Compile

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 AWS 접속 과정과 운용을 확인 하실 수 있습니다.
해당 예제의 MQTT 함수는 MQTT 데이터 통신 가이드에 활용된 함수가 그대로 활용되었으며, SSL 설정 및 파일시스템 접근 함수 등이 추가되었습니다.
> MQTT 예제에 활용된 함수들은 [MQTT 가이드][mbed-guide-bg96-mqtt] 에서 확인 하실 수 있습니다.

#### 4.1 Functions

```cpp
int8_t setMqttTlsEnable_BG96(bool enable);
```
* MQTT에서 SSL/TLS 보안 연결을 이용하도록 설정합니다.

```cpp
int8_t saveFileToStorage_BG96(char * path, const char * buf, int len);
```
* BG96 모듈의 파일 시스템에 지정된 path(경로+파일명)으로 buf의 내용을 저장합니다. len은 저장될 파일의 크기입니다.

```cpp
int8_t eraseFileStorageAll_BG96(void);
```
* BG96 모듈의 파일 시스템에 저장된 모든 파일을 삭제합니다.

```cpp
int8_t setTlsCertificatePath_BG96(char * param, char * path);
```
* SSL/TLS 연결에 이용할 인증서 정보를 등록합니다. param에는 인증서 종류, path는 파일시스템의 경로+파일명이 입력되어야 합니다.
  *  AWS 접속 시, Root CA, Client certificate, Client privatekey가 모두 설정되어야 합니다.

```cpp
int8_t setTlsConfig_sslversion_BG96(int ver);
```
* SSL/TLS 연결 시 버전을 설정합니다.
  * 0: SSL3.0
  * 1: TLS1.0
  * 2: TLS1.1
  * 3: TLS1.2
  * 4: ALL

```cpp
int8_t setTlsConfig_ciphersuite_BG96(char * ciphersuite);
```
* SSL/TLS 연결 시 이용할 Ciphersuite를 설정합니다. 다음과 같이 지정되어 있습니다.
  * 0X0035 TLS_RSA_WITH_AES_256_CBC_SHA
  * 0X002F TLS_RSA_WITH_AES_128_CBC_SHA
  * 0X0005 TLS_RSA_WITH_RC4_128_SHA
  * 0X0004 TLS_RSA_WITH_RC4_128_MD5
  * 0X000A TLS_RSA_WITH_3DES_EDE_CBC_SHA
  * 0X003D TLS_RSA_WITH_AES_256_CBC_SHA256
  * 0XC011 TLS_ECDHE_RSA_WITH_RC4_128_SHA
  * 0XC012 TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA
  * 0XC013 TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA
  * 0XC014 TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA
  * 0XC027 TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256
  * 0XC028 TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384
  * 0XC02F TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
  * 0XFFFF Support All

```cpp
int8_t setTlsConfig_seclevel_BG96(int seclevel);
```
* SSL/TLS 연결 시 authentication mode를 설정합니다.

```cpp
int8_t setTlsConfig_ignoreltime_BG96(bool enable);
```
* SSL/TLS 연결 시 veridity check 여부를 설정합니다.

#### 4.2 Demo

AWS IoT 인증서 및 사물 정보를 수정 후 예제를 실행하면 시리얼 터미널을 통해 다음과 같은 실행 결과를 볼 수 있습니다.

사물은 지정된 주기(예제 1분)마다 AWS IoT로 가상의 온도 정보 및 색깔 문자열을 Publish하여 shadow update를 요청합니다.
시작과 함께 shadow/update/accepted topic을 subscribe 했으므로, update 요청에 대한 응답 메시지를 수신 할 수 있습니다.

![][16]

Shadow의 update는 사물의 `활동`에서 그 내역을 확인 할 수 있습니다.

![][17]


[mbed-getting-started]: ./mbed_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/

[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-mqtt-an]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_MQTT_Application_Note_V1.0.pdf
[link-bg96-ssl-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_SSL_AT_Commands_Manual_V1.0.pdf
[link-bg96-file-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_FILE_AT_Commands_Manual_V1.0.pdf

[mbed-guide-bg96-mqtt]: ./mbed_guide_bg96_mqtt.md

[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png
[0]: ./imgs/mbed_guide_awsiot.jpg
[1]: ./imgs/mbed_guide_bg96_aws-1.png
[2]: ./imgs/mbed_guide_bg96_aws-2.png
[3]: ./imgs/mbed_guide_bg96_aws-3.png
[4]: ./imgs/mbed_guide_bg96_aws-4.png
[5]: ./imgs/mbed_guide_bg96_aws-5.png
[6]: ./imgs/mbed_guide_bg96_aws-6.png
[7]: ./imgs/mbed_guide_bg96_aws-7.png
[8]: ./imgs/mbed_guide_bg96_aws-8.png
[9]: ./imgs/mbed_guide_bg96_aws-9.png
[10]: ./imgs/mbed_guide_bg96_aws-10.png
[11]: ./imgs/mbed_guide_bg96_aws-11.png
[12]: ./imgs/mbed_guide_bg96_aws-12.png
[13]: ./imgs/mbed_guide_bg96_aws-13.png
[14]: ./imgs/mbed_guide_bg96_aws-14.png
[15]: ./imgs/mbed_guide_bg96_aws-15.png
[16]: ./imgs/mbed_guide_bg96_aws-16.png
[17]: ./imgs/mbed_guide_bg96_aws-17.png

