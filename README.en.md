# yoloTracker

> 🌐 [한국어 버전](README.md)

yoloTracker extends Darknet (YOLO) with object tracking, multi-camera streaming, and OpenMP-based parallel reception.
While the original YOLO only produces per-frame detection results, this repository assigns a persistent ID to each detected object and tracks it across frames.

## Changes from upstream Darknet/YOLO

### 1. Object ID tracking

Detected bounding boxes are matched to the same object in subsequent frames.
`opencv_tracker/` keeps IDs stable by combining:

- MOSSE tracker
- ORB feature matching
- Velocity-based position prediction
- History-based correction

Related files: `opencv_tracker/tracker_main.cpp`, `tracking_dot.cpp`, `orb.cpp`, `tracker.h`

### 2. Lightweight MOSSE-only tracker

`opencv_tracker_mosse/` is a simplified variant that uses only MOSSE.
It shares the same `out.txt` / `out.jpg` interface but has a simpler tracking strategy.

Related files: `opencv_tracker_mosse/tracker_main.cpp`, `tracking_dot.cpp`

### 3. Multi-camera input

1–4 network streams are merged into a single frame with `hconcat` / `vconcat` before being passed to YOLO.
The number of streams is controlled by the `STREAM` define.

Related files: `src/image_opencv.cpp`, `src/demo.c`, `include/darknet.h`

### 4. OpenMP parallel reception

`open_video_stream_cus()` receives and decodes JPEG buffers from multiple sockets in parallel.
Enable with `OPENMP=1` in the root `Makefile`.

Related files: `Makefile`, `src/image_opencv.cpp`

### 5. Video streaming tools

TCP-based camera send/receive utilities:

- `videoserver`: encodes camera frames as JPEG and streams them
- `videoclient`: receives a single stream
- `videoclient_4cam`: receives four streams

Related files: `video_streaming/videoserver.cpp`, `videoclient.cpp`, `videoclient_4cam.cpp`

### 6. Miscellaneous tools

- `ORB/`: ORB feature-matching experiments
- `video_maker/`: tool for generating result videos
- `nano_cam_on.cpp`, `nano_cam_on.sh`: scripts for launching the streaming server on a remote device (environment-specific)

## Directory layout

| Path | Role |
| --- | --- |
| `src/` | Modified Darknet core — multi-stream reception and frame merging |
| `opencv_tracker/` | Main tracker combining ORB, MOSSE, and prediction |
| `opencv_tracker_mosse/` | Lightweight MOSSE-only tracker |
| `video_streaming/` | Camera send/receive utilities |
| `ORB/` | ORB matching experiments |
| `video_maker/` | Result video generation tool |

## Pipeline

1. `videoserver` streams camera frames over the network.
2. The modified Darknet receives multiple streams and merges them into one frame.
3. YOLO detection results are written to `out.jpg` and `out.txt`.
4. `opencv_tracker` or `opencv_tracker_mosse` reads those files and assigns persistent IDs to each object.
5. Tracking results are sent to an external server or displayed on screen.

Detection results are handed off to the tracker via files (`out.txt` / `out.jpg`), not through a direct API call.

## Building

### Darknet core

Default `Makefile` flags:

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

Outputs: `darknet`, `libdarknet.a`, `libdarknet.so`

### Trackers and tools

```bash
cd opencv_tracker       && make tracker
cd opencv_tracker_mosse && make tracker
cd video_streaming      && make
cd ORB                  && make tracker
```

## Configuration checklist

### `CAM_NUM`, `STREAM` — `include/darknet.h`

```c
#define CAM_NUM 0
#define STREAM 4
```

Set `CAM_NUM` for local cameras or `STREAM` for network streams, depending on your setup.

### Streaming IP / port — `src/demo.c`, `src/image_opencv.cpp`

Camera server addresses and ports are hardcoded. Update them to match your environment.

### Tracking result server — `opencv_tracker/tracker_main.cpp`

The destination server address for tracking output is also hardcoded.

## Key customization points

### `src/image_opencv.cpp`

OpenCV I/O, multi-stream reception and merging, OpenMP parallel receive logic.

### `src/demo.c`

Connects the custom input source to the YOLO demo loop, initializes sockets, and writes `out.jpg` / `out.txt`.

### `opencv_tracker/`

Converts raw YOLO detections into tracked results: manages per-object tags and miss counts, handles motion prediction and screen-exit logic.

## Caveats

- IP addresses, ports, and server addresses are hardcoded throughout the source.
- The remote camera scripts are tied to a specific development environment.
- YOLO and the tracker communicate through files (`out.txt`, `out.jpg`), not a direct API.
- `DEBUG=1` is the default, so the build prioritizes debug convenience over performance.
- OpenMP optimization covers only the multi-stream reception path.
