# Darknet(YOLO) + tracker #
Darknet is an open source Object detecter written in C and CUDA. It's fast, easy to install, and supports CPU and GPU computation.

but it can't tracking the thing, which detected by yolo.
So we have created a system that can track each object by processing the information of the objects obtained from YOLO.

![yolo_vs_me](https://github.com/thsvkd/yoloTracker/blob/master/res/yolo_vs_me.png)
This is what we different between YOLO original and our project. 

In the existing YOLO, only the location and class of objects can be known, but our system can distinguish each object.
To make this possible, we used various methods such as ORB feature matching, histogram matching, and object velocity prediction. 

Our system consists of two algorithms.

1. one view object tracker.
2. muti view object tracker.



![HW_block_diagram](https://github.com/thsvkd/yoloTracker/blob/master/res/HW_block_diagram.png)

![entire_flow](https://github.com/thsvkd/yoloTracker/blob/master/res/entire_flow.png)
