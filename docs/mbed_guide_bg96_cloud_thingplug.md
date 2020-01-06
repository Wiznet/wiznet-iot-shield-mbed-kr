# MBED 기반의 Cat.M1 ThingPlug 연동 가이드(2019.6.1 이후 신규 회원 가입 중단)

## 목차

-   [시작하기 전에](#Prerequisites)
-   [소개](#Step-1-Overview)
-   [동작 구조 예제](#Step-2-SampleCode)
-   [예제 코드 빌드 및 실행](#Step-3-Build-and-Run)

<a name="Prerequisites"></a>
## 시작하기 전에

> 본 문서에서는 Arm MBED 기반 개발 환경에서 WIZnet IoT shield와 ST Nucleo-L476RG 보드를 이용하여 Cat.M1 단말의 **[ThingPlug][link-thingplug-portal]** 에 연결하고 데이터를 송신하는 방법에 대한 가이드를 제공합니다.
> 
> Cat.M1과 같은 Cellular IoT 디바이스는 통신 서비스 사업자의 운영 기준 및 규정에 따라 모듈 펌웨어 및 동작 방식에 차이가 있을 수 있습니다. 본 문서는 한국 **[SK Telecom Cat.M1 서비스][skt-iot-portal]** 를 기준으로 작성되었습니다.
> 
> 아래 가이드는 Arm MBED 기반에서 Cat.M1 모듈이 ThingPlug 연동하는 내용에 대해 설명하고 있습니다. ThingPlug 사용법에 대한 보다 자세한 설명은 ~를 확인하세요

### Development Environment
* **[SKT ThingPlug Portal][link-thingplug-portal]**
* **[Arm MBED Web Compiler][link-mbed-compiler]**



### Supported Boards

| MCU Board | IoT Shield Interface Board |
|:--------:|:--------:|
| [ST Nucleo-L476RG][link-nucleo-l476rg] | WIoT-QC01 (BG96) |

<a name="Step-1-Overview"></a>
## 소개

**[ThingPlug][link-thingplug-portal]** 는 고객의 IoT 디바이스의 관제/제어와 빅데이터 비즈니스를 제공하는 SKT의 IoT 플랫폼입니다. 사용자가 ThingPlug를 활용하여 IoT 서비스를 손쉽게 구현할 수 있도록 지원하고 있습니다.

![][1]

Cat.M1 모듈 및 외장형 모뎀은 UART 인터페이스를 통해 활용하는 AT 명령어로 제어하는 것이 일반적입니다. 또한 SKT 인증 Cat.M1 모듈에는 ThingPlug와 쉽게 연동할 수 있는 AT 명령어가 구현되어 있고, 이 명령어를 사용해 쉽게 ThingPlug와 연결할 수 있습니다.

이번 ThingPlug 연동 예제는 Arm MBED 환경에서 ThingPlug AT 명령어를 활용한 Cat M1 모듈의 데이터 송수신을 확인하실 수 있습니다.

<a name="Step-2-SampleCode"></a>

## 동작 구조 예제 (Cat.M1 ThingPlug 접속)

```
AT+SKTPCON
status: disconnected   // 초기 상태 disconnected

OK
AT+SKTPCON=1,"MQTT","test.sktiot.com",1883,120,1,"simple_v1","a0149f60b*********","svc01","dev02"
OK

+SKTPCON: 0            // 접속에 성공하면  +SKTPCON: 0 수신
AT+SKTPCON
status: connected      // 접속 상태가 connected로 변경됨

OK
AT+SKTP                // 연결 정보 확인
version: 1.0.0
status: connected
protocol: MQTT
host: test.sktiot.com
port: 1883
keep-alive: 120
cleansession: 1
api_version: simple_v1
device_token: ....b1dc
service_id: svc01
device_id: dev02
client_id: 866425035073255_dev02

OK

```


## 동작 구조 예제 (Cat.M1 ThingPlug 데이터 전송)

```
AT+SKTPDAT=1,"telemetry",0          // 데이터 타입 telemetry로 전송시
> {"temperature":12, "humidity":34} // '>' 프롬프트 이후에 데이터 입력, 입력 완료는 Ctrl+Z
OK

AT+SKTPDAT=1,"attribute",0          // 데이터 타입 attribute로 전송시
> {"LED":"ON"}                      // '>' 프롬프트 이후에 데이터 입력, 입력 완료는 Ctrl+Z
OK

```


## 동작 구조 예제 (Cat.M1 ThingPlug 제어명령 수신)

```

+SKTPCMD: tp_user,1815296803,1,[20,14]  // RPC 수신
AT+SKTPRES=1,"tp_user",1815296803,0     // PRC에 대한 처리 결과 응답
> {"result":"success"}
OK

+SKTPCMD: tp_user,1815296803,3

+SKTPCMD: tp_user,1815296803,4

```


<a name="Step-3-Build-and-Run"></a>
## 예제 코드 빌드 및 실행

### 1. Import project
MBED 온라인 컴파일러에 Log in 하여 상단 메뉴의 `Import`를 클릭합니다.

`Programs` 탭을 클릭 후 검색창에 다음 예제를 검색하여 나의 Program Workspace에 추가합니다.


| Sample code | Link |
|:--------|:--------|
| WIZnet-IoTShield-BG96-ThingPlug | https://os.mbed.com/users/hkjung/code/WIZnet-IoTShield-BG96-ThingPlug/ |


> `Import Programs` 팝업 창이 활성화 될 때, Import As 옵션이 Program으로 설정되어 있어야 합니다.

![][import1]

링크를 클릭하여 예제 프로젝트의 저장소를 방문한 경우, 오른쪽 상단 `Repository toolbox`의 `Import into Compiler`를 클릭하시면 나의 Program Workspace 해당 예제를 추가 할 수 있습니다.s

![][import2]

### 2. Modify parameters

ThingPlug 연동을 위해 서비스ID, 디바이스ID, 디바이스Token과 디바이스 명세에 등록한 Telemetry, attribute를 변경해야 합니다.

````cpp
/* THINGPLUG PRIVATE*/
#define THINGPLUG_SVC_NAME              "svc01"
#define THINGPLUG_DEV_ID                "dev02"
#define THINGPLUG_DEV_TOKEN             "a0149f60b*********"

#define THINGPLUG_TELEMETRY1            "temperature"
#define THINGPLUG_TELEMETRY2            "humidity"
#define THINGPLUG_ATTRIBUTE             "LED"
````

### 3. Compile

상단 메뉴의 `Complie` 버튼을 클릭합니다.

![][compile]

컴파일이 성공적으로 완료되면 `project_name.bin` 파일이 생성되어 download 됩니다. 이 파일을 `Drag & Drop`하여 연결된 MBED target 보드에 삽입하면 firmware write가 완료됩니다.

### 4. Run

예제 샘플 코드를 통해 Cat.M1 모듈의 ThingPlug 연결과 데이터 송/수신을 테스트 해볼 수 있습니다.

> * 예제 샘플 코드는 보드가 부팅 되면, 미리 정의된 ThingPlug 정보를 바탕으로 ThingPlug에 접속합니다. 접속이 완료되면 telemetry와 attribute 데이터를 전송하고 제어명령 수신 상태로 동작하게 됩니다.

#### 4.1 Functions

```cpp
int8_t connectThingPlug_BG96(char* svc_id, char* dev_id, char* dev_token);
```
 * ThingPlug에 접속합니다.
 * ThingPlug에 접속하기 위한 파라메터인 서비스ID와 다비아스ID, 디바이스Token을 전달합니다.

```cpp
int8_t sendThingPlugData_Json_BG96(char* type, char * data, int len);
```
 * ThingPlug로 Json 타입의 데이터를 전송합니다.
 * 데이터 타입(telemetry, attribute)와 데이터, 데이터의 길이를 파라메터로 전달합니다.

```cpp
int8_t recvThingPlugCmd_BG96(char* cmd_type, unsigned int* rpc_id, char* msg);
```
  * SMS 메시지의 수신 여부를 체크합니다. 만약 메시지가 수신되면 수신 메시지 index를 리턴합니다.
  * ThingPlug의 제어명령 수신 여부를 체크합니다. 만약 제어명령이 있다면, true를 리턴합니다.
  * 메인 루프 안에서 동작해야 합니다.

```cpp
int8_t sendThingPlugResp_BG96(char* cmd_type, unsigned int rpc_id, int result, char* result_data, int result_len);
```
 * 수신한 제어명령의 수행 결과를 ThingPlug로 전달합니다.
 * recvThingPlugCmd_BG96()에서 수신한 제어명령의 cmd_type과 rpc_id를 수행결과의 파라메터로 전달해야 합니다.

#### 4.2 Demo

* 접속 후 telemetry 1회 전송

![][3]

* 제어명령 RPC 수신 후 결과 전송

![][4]


[mbed-getting-started]: ./mbed_get_started.md
[skt-iot-portal]: https://www.sktiot.com/iot/developer/guide/guide/catM1/menu_05/page_01
[link-mbed-compiler]: https://ide.mbed.com/compiler/
[link-nucleo-l476rg]: https://os.mbed.com/platforms/ST-Nucleo-L476RG/
[link-bg96-atcommand-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_AT_Commands_Manual_V2.1.pdf
[link-bg96-tcp-manual]: https://www.quectel.com/UploadImage/Downlad/Quectel_BG96_TCP(IP)_AT_Commands_Manual_V1.0.pdf

[import1]: ./imgs/mbed_guide_webide_import.png
[import2]: ./imgs/mbed_guide_webide_import_repo.png
[compile]: ./imgs/mbed_guide_webide_compile.png
[1]: ./imgs/thingplug_main.png
[3]: ./imgs/thingplug_mbed_data_send.PNG
[4]: ./imgs/thingplug_mbed_cmd_result.PNG

[link-thingplug-atcommand-manual]: https://www.sktiot.com/api/common/file/download?fileId=00EYNW8OXF8VA94EE29A
[link-thingplug-portal]: https://portal.sktiot.com/intro
 
