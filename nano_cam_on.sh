#!/bin/bash

thsvkd_cam_on="sshpass -p thsxogud1 ssh -o StrictHostKeyChecking=no thsvkd@114.70.22.21 -p4000 ./yoloTracker/video_streaming/videoserver 5000; sshpass -p thsxogud1 ssh -o StrictHostKeyChecking=no thsvkd1@114.70.22.21 -p4001 ./yoloTracker/video_streaming/videoserver 5001; sshpass -p thsxogud1 ssh -o StrictHostKeyChecking=no thsvkd2@114.70.22.21 -p4002 ./yoloTracker/video_streaming/videoserver 5002; sshpass -p thsxogud1 ssh -o StrictHostKeyChecking=no thsvkd3@114.70.22.21 -p4003 ./yoloTracker/video_streaming/videoserver 5003"

kill0="pkill -x videoserver"

if [ "$1" = "1" ]
then
    ${thsvkd_cam_on}
else
    ${kill0}
fi
