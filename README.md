# yoloTracker

yoloTracker는 **Darknet(YOLO)** 을 기반으로, 이 저장소에서 추가한 **객체 추적**·**멀티 카메라 스트리밍**·**병렬 수신 최적화**에 초점을 맞춘 프로젝트입니다.  
원본 YOLO가 프레임 단위 검출 결과만 제공하는 반면, 이 프로젝트는 **객체별 ID를 유지하면서 연속 프레임을 추적**하는 기능을 추가합니다.

## 원본 Darknet/YOLO 대비 변경점

### 1. 객체별 ID 유지가 가능한 추적 기능 추가
- 검출된 박스를 다음 프레임에서도 같은 객체로 이어서 추적합니다.
- `opencv_tracker/`는 다음 방법을 조합해 객체를 유지합니다.
  - **MOSSE tracker**
  - **ORB feature matching**
  - **속도 기반 위치 예측**
  - 일부 로직에서 **히스토리 기반 보정**
- 결과적으로 단순 검출이 아니라 **“같은 사람/같은 물체를 계속 같은 ID로 관리”** 하는 흐름을 목표로 합니다.

관련 파일:
- `/home/runner/work/yoloTracker/yoloTracker/opencv_tracker/tracker_main.cpp`
- `/home/runner/work/yoloTracker/yoloTracker/opencv_tracker/tracking_dot.cpp`
- `/home/runner/work/yoloTracker/yoloTracker/opencv_tracker/orb.cpp`
- `/home/runner/work/yoloTracker/yoloTracker/opencv_tracker/tracker.h`

### 2. MOSSE 기반 경량 추적 버전 분리
- `opencv_tracker_mosse/`는 보다 단순한 **MOSSE 중심 추적기**입니다.
- 동일한 `out.txt`, `out.jpg` 기반으로 동작하지만, 추적 전략은 `opencv_tracker/`보다 단순합니다.

관련 파일:
- `/home/runner/work/yoloTracker/yoloTracker/opencv_tracker_mosse/tracker_main.cpp`
- `/home/runner/work/yoloTracker/yoloTracker/opencv_tracker_mosse/tracking_dot.cpp`

### 3. 멀티 카메라 영상 수집/합성 기능 추가
- 원본 Darknet의 단일 입력 흐름 외에, 네트워크로 들어오는 **여러 카메라 스트림을 수신**하도록 확장했습니다.
- `STREAM` 값에 따라 1~4개 영상을 받아 하나의 프레임으로 합쳐 YOLO 입력으로 사용합니다.
- `src/image_opencv.cpp`에서 `hconcat`, `vconcat`으로 다중 영상을 결합합니다.

관련 파일:
- `/home/runner/work/yoloTracker/yoloTracker/src/image_opencv.cpp`
- `/home/runner/work/yoloTracker/yoloTracker/src/demo.c`
- `/home/runner/work/yoloTracker/yoloTracker/include/darknet.h`

### 4. OpenMP 기반 병렬 수신 코드 추가
- 현재 코드베이스의 최근 변경점은 **OpenMP를 이용한 다중 스트림 병렬 수신**입니다.
- 루트 `Makefile`에서 `OPENMP=1`을 켜고 `-fopenmp`를 추가합니다.
- `open_video_stream_cus()`에서 여러 소켓의 JPEG 버퍼를 병렬로 받아 디코딩합니다.

관련 파일:
- `/home/runner/work/yoloTracker/yoloTracker/Makefile`
- `/home/runner/work/yoloTracker/yoloTracker/src/image_opencv.cpp`

### 5. 영상 스트리밍 도구 추가
- `video_streaming/`에는 TCP 기반 영상 송수신 도구가 포함되어 있습니다.
- `videoserver`는 카메라 프레임을 JPEG로 인코딩해 전송합니다.
- `videoclient`와 `videoclient_4cam`은 이를 수신해 표시하거나 다중 입력을 다룹니다.

관련 파일:
- `/home/runner/work/yoloTracker/yoloTracker/video_streaming/videoserver.cpp`
- `/home/runner/work/yoloTracker/yoloTracker/video_streaming/videoclient.cpp`
- `/home/runner/work/yoloTracker/yoloTracker/video_streaming/videoclient_4cam.cpp`

### 6. ORB 실험 코드 및 보조 도구 추가
- `ORB/`는 ORB 정합 실험용 코드입니다.
- `video_maker/`는 결과 영상을 만드는 보조 도구입니다.
- `nano_cam_on.cpp`, `nano_cam_on.sh`는 원격 장비에서 스트리밍 서버를 띄우기 위한 환경 의존 스크립트입니다.

## 저장소에서 추가된 주요 구성 요소

| 경로 | 역할 |
| --- | --- |
| `/home/runner/work/yoloTracker/yoloTracker/src/` | Darknet 본체 수정본. 다중 스트림 수신과 프레임 결합 포함 |
| `/home/runner/work/yoloTracker/yoloTracker/opencv_tracker/` | ORB/MOSSE/예측을 조합한 메인 추적기 |
| `/home/runner/work/yoloTracker/yoloTracker/opencv_tracker_mosse/` | MOSSE 중심 추적기 |
| `/home/runner/work/yoloTracker/yoloTracker/video_streaming/` | 영상 송수신 유틸리티 |
| `/home/runner/work/yoloTracker/yoloTracker/ORB/` | ORB 정합 실험 코드 |
| `/home/runner/work/yoloTracker/yoloTracker/video_maker/` | 결과 영상 생성 도구 |

## 동작 흐름

1. `video_streaming/videoserver`가 카메라 프레임을 네트워크로 전송합니다.
2. Darknet 수정본이 여러 스트림을 받아 하나의 입력 프레임으로 합칩니다.
3. YOLO 검출 결과를 `out.jpg`, `out.txt`로 저장합니다.
4. `opencv_tracker/` 또는 `opencv_tracker_mosse/`가 이를 읽어 객체별 ID를 붙여 추적합니다.
5. 추적 결과는 별도 서버로 전송하거나 화면에 시각화합니다.

즉, 이 프로젝트의 핵심은 **YOLO 검출 → 파일 기반 전달 → 후처리 추적기에서 ID 부여 및 유지** 입니다.

## 빌드

### 1. Darknet 본체
루트 `Makefile` 기본 설정:

```make
GPU=1
CUDNN=1
OPENCV=1
OPENMP=1
DEBUG=1
```

빌드:

```bash
cd /home/runner/work/yoloTracker/yoloTracker
make
```

생성물:
- `darknet`
- `libdarknet.a`
- `libdarknet.so`

### 2. 메인 추적기

```bash
cd /home/runner/work/yoloTracker/yoloTracker/opencv_tracker
make tracker
```

### 3. MOSSE 추적기

```bash
cd /home/runner/work/yoloTracker/yoloTracker/opencv_tracker_mosse
make tracker
```

### 4. 스트리밍 도구

```bash
cd /home/runner/work/yoloTracker/yoloTracker/video_streaming
make
```

### 5. ORB 실험 도구

```bash
cd /home/runner/work/yoloTracker/yoloTracker/ORB
make tracker
```

## 실행 전 확인할 설정 포인트

### `CAM_NUM`, `STREAM`
- `/home/runner/work/yoloTracker/yoloTracker/include/darknet.h`
- 현재 기본값은 다음과 같습니다.

```c
#define CAM_NUM 0
#define STREAM 4
```

- 로컬 카메라를 직접 붙일지, 네트워크 스트림을 받을지에 따라 값을 조정해야 합니다.

### 스트리밍 대상 IP / 포트
- `/home/runner/work/yoloTracker/yoloTracker/src/demo.c`
- `/home/runner/work/yoloTracker/yoloTracker/src/image_opencv.cpp`
- 현재 카메라 서버 주소와 포트가 코드에 하드코딩되어 있습니다.

### 추적 결과 전송 서버
- `/home/runner/work/yoloTracker/yoloTracker/opencv_tracker/tracker_main.cpp`
- 외부 서버 주소가 코드에 고정되어 있어 실행 환경에 맞게 수정이 필요할 수 있습니다.

## 이 프로젝트에서 특히 봐야 할 커스텀 포인트

### `src/image_opencv.cpp`
- OpenCV 기반 입출력 처리
- 멀티 스트림 수신
- 수신 영상 합성
- OpenMP 병렬 수신 로직

### `src/demo.c`
- YOLO demo 루프에서 커스텀 입력 소스 연결
- 소켓 초기화
- `out.jpg`, `out.txt` 저장 흐름과 연동

### `opencv_tracker/`
- YOLO 결과를 실제 추적 결과로 바꾸는 핵심 로직
- 객체별 tag, miss count, 이동 예측, 화면 이탈 처리 포함

### `video_streaming/`
- 카메라 영상을 네트워크로 송수신하기 위한 별도 유틸리티

## 제한 사항 / 주의 사항

- 여러 IP, 포트, 서버 주소가 코드에 **하드코딩**되어 있습니다.
- 원격 카메라 실행 보조 스크립트는 **특정 개발 환경에 강하게 의존**합니다.
- YOLO와 추적기 사이가 직접 API 호출이 아니라 **`out.txt`, `out.jpg` 파일 기반**으로 연결됩니다.
- `DEBUG=1`이 기본이라 성능보다 디버깅 편의에 맞춰져 있습니다.
- OpenMP 최적화는 현재 **다중 스트림 수신 경로**에 집중되어 있습니다.

## 정리

이 저장소는 단순히 Darknet을 가져온 것이 아니라, 다음 기능을 덧붙인 **YOLO 기반 추적 시스템**입니다.

- 다중 객체 추적
- 객체 ID 유지
- 멀티 카메라 입력 처리
- 네트워크 영상 스트리밍
- OpenMP 기반 병렬 수신 최적화

따라서 이 프로젝트를 이해할 때는 원본 YOLO 사용법보다, **이 저장소에서 추가한 입력 파이프라인과 추적 파이프라인**을 중심으로 보는 것이 중요합니다.
