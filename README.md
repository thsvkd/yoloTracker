# Darknet(YOLO) + tracker #
Darknet is an open source Object detecter written in C and CUDA. It's fast, easy to install, and supports CPU and GPU computation.

but it can't tracking the thing, which detected by yolo.
So we have created a system that can track each object by processing the information of the objects obtained from YOLO.

![yolo_vs_me](https://user-images.githubusercontent.com/53033449/69026547-b292b380-0a0e-11ea-998c-92d25f188a8a.png)
This is what we different between YOLO original and our project. 

In the existing YOLO, only the location and class of objects can be known, but our system can distinguish each object.
To make this possible, we used various methods such as ORB feature matching, histogram matching, and object velocity prediction. 

Our system consists of two algorithms.

    1. one view object tracker.
   
    2. muti view object tracker.


![HW_block_diagram](https://user-images.githubusercontent.com/53033449/69026531-9b53c600-0a0e-11ea-92b3-d3beed49f561.png)
![entire_flow](https://user-images.githubusercontent.com/53033449/69026500-719a9f00-0a0e-11ea-8e2e-4663d24ec6e4.png)
