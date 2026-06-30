# yoloTracker

> 🌐 [English version](README.en.md)

Darknet(YOLO) 위에 객체 추적, 멀티 카메라 스트리밍, OpenMP 병렬 수신을 얹은 프로젝트입니다.
원본 YOLO는 프레임마다 검출 결과를 내놓는 데서 끝나지만, 이 저장소는 검출된 객체에 ID를 붙여 연속 프레임에 걸쳐 추적합니다.

## 원본 Darknet/YOLO 대비 변경점

### 1. 객체 ID 추적

검출된 박스를 이후 프레임에서도 같은 객체로 이어 추적합니다.
`opencv_tracker/`는 다음 방법을 조합해 ID를 유지합니다.

- MOSSE tracker
- ORB feature matching
- 속도 기반 위치 예측
- 히스토리 기반 보정

관련 파일: `opencv_tracker/tracker_main.cpp`, `tracking_dot.cpp`, `orb.cpp`, `tracker.h`

### 2. MOSSE 전용 경량 추적기

`opencv_tracker_mosse/`는 MOSSE만 사용하는 단순화된 버전입니다.
`out.txt` / `out.jpg` 인터페이스는 동일하고, 추적 전략이 더 가볍습니다.

관련 파일: `opencv_tracker_mosse/tracker_main.cpp`, `tracking_dot.cpp`

### 3. 멀티 카메라 입력

네트워크로 들어오는 1~4개 스트림을 `hconcat` / `vconcat`으로 합쳐 YOLO 입력으로 씁니다.
`STREAM` 값으로 스트림 수를 지정합니다.

관련 파일: `src/image_opencv.cpp`, `src/demo.c`, `include/darknet.h`

### 4. OpenMP 병렬 수신

`open_video_stream_cus()`에서 여러 소켓의 JPEG 버퍼를 병렬로 받아 디코딩합니다.
루트 `Makefile`에서 `OPENMP=1`로 활성화합니다.

관련 파일: `Makefile`, `src/image_opencv.cpp`

### 5. 영상 스트리밍 도구

TCP 기반 카메라 송수신 도구입니다.

- `videoserver`: 카메라 프레임을 JPEG로 인코딩해 전송
- `videoclient`: 단일 스트림 수신
- `videoclient_4cam`: 4채널 수신

관련 파일: `video_streaming/videoserver.cpp`, `videoclient.cpp`, `videoclient_4cam.cpp`

### 6. 기타 보조 도구

- `ORB/`: ORB 정합 실험용 코드
- `video_maker/`: 결과 영상 생성 도구
- `nano_cam_on.cpp`, `nano_cam_on.sh`: 원격 장비에서 스트리밍 서버를 띄우는 스크립트 (환경 의존)

## 디렉터리 구조

| 경로 | 역할 |
| --- | --- |
| `src/` | Darknet 수정본. 멀티 스트림 수신·프레임 합성 포함 |
| `opencv_tracker/` | ORB/MOSSE/예측 조합 메인 추적기 |
| `opencv_tracker_mosse/` | MOSSE 전용 경량 추적기 |
| `video_streaming/` | 카메라 영상 송수신 유틸리티 |
| `ORB/` | ORB 정합 실험 코드 |
| `video_maker/` | 결과 영상 생성 도구 |

## 동작 흐름

1. `videoserver`가 카메라 프레임을 네트워크로 전송
2. Darknet 수정본이 여러 스트림을 받아 하나의 프레임으로 합성
3. YOLO 검출 결과를 `out.jpg` / `out.txt`로 저장
4. `opencv_tracker` 또는 `opencv_tracker_mosse`가 파일을 읽어 객체마다 ID를 붙여 추적
5. 추적 결과를 외부 서버로 전송하거나 화면에 출력

YOLO 검출 결과는 `out.txt` / `out.jpg` 파일을 통해 추적기에 전달됩니다.

## 빌드

### Darknet 본체

기본 `Makefile` 설정:

```make
GPU=1
CUDNN=1
OPENCV=1
OPENMP=1
DEBUG=1
```

```bash
make
```

생성물: `darknet`, `libdarknet.a`, `libdarknet.so`

### 추적기 / 도구

```bash
cd opencv_tracker       && make tracker
cd opencv_tracker_mosse && make tracker
cd video_streaming      && make
cd ORB                  && make tracker
```

## 실행 전 확인할 설정

### `CAM_NUM`, `STREAM` — `include/darknet.h`

```c
#define CAM_NUM 0
#define STREAM 4
```

로컬 카메라를 쓸 경우 `CAM_NUM`을, 네트워크 스트림을 받을 경우 `STREAM`을 조정합니다.

### 스트리밍 IP / 포트 — `src/demo.c`, `src/image_opencv.cpp`

카메라 서버 주소와 포트가 하드코딩되어 있습니다. 환경에 맞게 수정해야 합니다.

### 추적 결과 전송 서버 — `opencv_tracker/tracker_main.cpp`

외부 서버 주소가 하드코딩되어 있습니다.

## 주요 커스텀 포인트

### `src/image_opencv.cpp`

OpenCV 입출력, 멀티 스트림 수신·합성, OpenMP 병렬 수신 로직이 모여 있습니다.

### `src/demo.c`

YOLO demo 루프에서 커스텀 입력 소스를 연결하고, 소켓을 초기화하며 `out.jpg` / `out.txt`를 씁니다.

### `opencv_tracker/`

YOLO 결과를 받아 객체별 tag와 miss count를 관리하고, 이동 예측과 화면 이탈 처리를 수행합니다.

## 주의 사항

- IP, 포트, 서버 주소가 코드에 하드코딩되어 있습니다.
- 원격 카메라 스크립트는 특정 개발 환경에 종속됩니다.
- YOLO와 추적기 사이는 API가 아니라 파일(`out.txt`, `out.jpg`)로 연결됩니다.
- `DEBUG=1`이 기본값이므로 성능보다 디버깅 편의를 우선합니다.
- OpenMP 최적화는 다중 스트림 수신 경로에만 적용되어 있습니다.
