# :pushpin: Ransomware 복구도구 검증 소프트웨어 개발 프로젝트
> 입력받은 랜섬웨어 복구도구를 VirtaulBox로 구축한 시스템 내에서 테스트하는 프로젝트.

</br>

## 1. 제작 기간 & 참여 인원
- 2019년 03월 ~ 2019년 05월
- 개인 프로젝트

</br>

## 2. 사용 기술
- C++
- MFC
- Windows의 Crypto API

#### 개발 환경
    - Visual studio Community 2019
    - VirtualBox 5.2.26

</br>

## 3. 폴더 구조

```text
├── Agent_v3.0      (VM 이미지 내부에서 해당 랜섬웨어와 복구도구를 받아서 테스트하는 프로그램)
├── Common          (Agent와 Manager에서 공통으로 쓰이는 데이터 타입)
└── Manager_v3.4    (VM 이미지를 관리하고 테스트 결과 출력하는 프로그램)
```

## 4. 핵심 기능
- VirtualBox에서 제공하는 명령어로 Command Line에서 Virtual Machine을 제어하는 기능 구현.
- Multi Thread로 다대일 TCP 통신 구현.
- MFC로 VirtualBox에 명령을 내리는 프로그램의 UI 개발.
- 감염되기 전파일의 Hash 값과 복구된 파일의 Hash 값을 비교하여 복구 여부를 판별하는 기능 구현.
- VirtualBox로 폐쇄적인 네트워크 구성.
- 결과 보고서를 CSV 파일 형식으로 출력하는 기능 구현.
