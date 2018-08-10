# Detect-Track
Four tracking algorithms(Staple and Template) organized with cascade classifier for object detection.
- **Video Show**
  - http://v.youku.com/v_show/id_XMzYxODcyNjEzMg==.html
  - http://v.youku.com/v_show/id_XMzYxODcxODY2OA==.html
  - https://youtu.be/zZZoJIJmQd8
  - https://youtu.be/rImqJhreMy4
  
! Requires: OpenCV > 3.1.0

## [cascade_staple](cascade_staple)
- 检测器：级联分类器
- 跟踪器：staple

## [cascade_TM](cascade_TM)
- 检测器：级联分类器
- 跟踪器：Template tracker

## [yolo_TM](yolo_TM)
- Requires: 自行编译[darknet](https://github.com/AlexeyAB/darknet)，将生成的darknet.so复制到/usr/lib/目录下
- 检测器：YOLOv3
- 跟踪器：Template tracker

## [yolo_KCF](yolo_KCF)
- Requires: Compiled with OpenCV_contrib
- Requires: 自行编译[darknet](https://github.com/AlexeyAB/darknet)，将生成的darknet.so复制到/usr/lib/目录下
- 检测器：YOLOv3
- 跟踪器：KCF

## [track_TM](track_TM)
- 跟踪器：Template tracker

## [track_staple](track_staple)
- 跟踪器：staple

## [track_opencv](track_opencv)
- Requires: Compiled with OpenCV_contrib
- 跟踪器："BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN", "CSRT"

## [color_detect](color_detect)
- 颜色检测器

## [serial_linux](serial_linux)
- linux串口通信封装

## [serial_windows](serial_windows)
- windows串口通信封装
