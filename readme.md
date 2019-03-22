# WIZnet IoT Shield for Arm MBED

이 저장소에는 다음 자료들이 포함되어 있습니다.
* [Arm MBED](https://www.mbed.com/) 기반으로 Cat.M1 응용 개발 환경을 구축하는 **개발 시작 가이드**
* Cat.M1 장치의 TCP 데이터 송수신, SMS, GPS, 클라우드 서비스 연동 등에 대한 **가이드 문서**
* [Arm MBED Web IDE](https://ide.mbed.com/compiler/) 상에서 동작하는 **예제 코드**

![][hw-stack-nucleo]

## Documentation
개발 시작 가이드 및 응용 가이드 문서는 본 저장소의 **[Wiki](https://github.com/Wiznet/wiznet-iot-shield-mbed-kr/wiki/)** 에서 확인 할 수 있습니다.


## Folder Structure

### /docs
Arm MBED 플랫폼을 기반으로 WIZnet IoT shield의 Cat.M1 통신을 활용하는 응용 장치 구현 시 참고할 수 있는 다양한 문서가 위치합니다.
* imgs: 문서에서 활용된 이미지들이 저장된 폴더입니다.

### /samples/catm1
Arm MBED 플랫폼의 Web IDE에서 확인 할 수 있는 예제 코드가 위치합니다. 각 Cat.M1 모듈 벤더 별로 나뉘어 있으며, 기능 동작을 위한 AT command 기반으로 구현되어 있습니다.
* WIoT-QC01: 앰투앰넷 BG96(Quectel) 모듈의 예제 코드
* WIoT-WM01: 우리넷 WM-N400MSE 모듈의 예제 코드
* WIoT-AM01: AM텔레콤 AMM5918K 모듈의 예제 코드

## Key features and Roadmap

### Cat.M1 Samples and Docs
:heavy_check_mark: 활용 가능  :heavy_multiplication_x: 지원 예정이며, 준비 중  :heavy_minus_sign: 지원 예정 없음


|        Title       |         Doc        |      BG96 code     |      WM-N400MSE code     |       AMM5918K code      |                            Description                           |
|:------------------:|:------------------:|:------------------:|:------------------------:|:------------------------:|:----------------------------------------------------------------:|
| 개발 시작 가이드   | :heavy_check_mark: | :heavy_minus_sign: |    :heavy_minus_sign:    |    :heavy_minus_sign:    | 초기 개발환경 구축                                               |
| Ping 테스트        | :heavy_minus_sign: | :heavy_check_mark: |    :heavy_multiplication_x:    |    :heavy_multiplication_x:    | 네트워크 상의 지정된 목적지와 Ping 테스트 예제                   |
| TCP/IP 데이터 통신 | :heavy_check_mark: | :heavy_check_mark: |    :heavy_multiplication_x:    |    :heavy_multiplication_x:    | TCP client로 동작하여 데이터 송신 및 수신하는 예제               |
| GPS 활용하기       | :heavy_check_mark: | :heavy_check_mark: |    :heavy_multiplication_x:    |    :heavy_multiplication_x:    | Cat.M1 모듈에 내장된 GPS 위치 측위 예제                          |
| SMS 활용하기       | :heavy_check_mark: | :heavy_check_mark: |    :heavy_multiplication_x:    | :heavy_multiplication_x: | Cat.M1 모듈에서 지원하는 SMS(단문 메시지 서비스)의 활용 예제     |
| PSM 저전력 모드    | :heavy_check_mark: | :heavy_check_mark: |    :heavy_multiplication_x:    | :heavy_multiplication_x: | Cat.M1 모듈에서 지원하는 저전력 모드(PSM) 활용 예제              |
| ThingPlug 연동하기 | :heavy_check_mark: | :heavy_check_mark: | :heavy_multiplication_x: | :heavy_multiplication_x: | SKT의 클라우드 서비스인 ThingPlug 연동 예제와 Widget 활용 가이드 |
| AWS 연동하기       | :heavy_check_mark: | :heavy_multiplication_x: |    :heavy_minus_sign:    |    :heavy_minus_sign:    | 대표적인 글로벌 클라우드 서비스인 Amazon AWS 연동 예제           |
| Azure 연동하기     | :heavy_multiplication_x: | :heavy_multiplication_x: |    :heavy_minus_sign:    |    :heavy_minus_sign:    | 대표적인 글로벌 클라우드 서비스인 MS Azure 연동 예제             |

## Other WIZnet IoT Shield Repository
**[wiznet-iot-shield](https://github.com/Wiznet/wiznet-iot-shield-kr/)** 저장소를 방문하면 다른 플랫폼 보드를 기반으로 동작하는 다양한 예제에 대한 저장소 리스트를 확인 할 수 있습니다.


## Support

![][forum]

**[WIZnet Developer Forum](https://forum.wiznet.io/)** 을 통해 전세계의 WIZnet 기술 전문가들에게 질문하고 의견을 전달할 수 있습니다.
지금 방문하세요.

## License
**WIZnet IoT Shield for Arm MBED** 저장소의 모든 문서와 예제는 [Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0)으로 배포됩니다.



[forum]: ./docs/imgs/forum.jpg
[hw-stack-nucleo]: ./docs/imgs/hw/WIoT-Shield-QC01-Nucleo-L476RG.png


