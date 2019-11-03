#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    string video_path[4];
    VideoCapture cap[4];
    VideoWriter writer;

    for (int i = 0; i < 4; i++)
        video_path[i] = string(argv[i + 1]);

    for (int i = 0; i < 4; i++)
    {
        cap[i] = VideoCapture(video_path[i]);
        if (!cap[i].isOpened())
        {
            cerr << "에러 - 카메라를 열 수 없습니다.\n";
            return -1;
        }
    }

    Size size = Size((int)cap[1].get(CAP_PROP_FRAME_WIDTH) * 2, (int)cap[0].get(CAP_PROP_FRAME_HEIGHT) * 2);

    double fps = 60.0;
    writer.open("output.mp4", VideoWriter::fourcc('X', 'V', 'I', 'D'), fps, size, true);

    if (!writer.isOpened())
    {
        cout << "동영상을 저장하기 위한 초기화 작업 중 에러 발생" << endl;
        return 1;
    }

    Mat frame[4];
    Mat result;

    while (1)
    {
        cap[0].read(frame[0]);
        cap[1].read(frame[1]);
        cap[2].read(frame[2]);
        cap[3].read(frame[3]);

        for (int i = 0; i < 4; i++)
        {
            if (frame[i].empty())
            {
                cerr << "빈 영상이 캡쳐되었습니다.\n";
                break;
            }
        }

        hconcat(frame[0], frame[1], frame[0]);
        hconcat(frame[2], frame[3], frame[2]);
        vconcat(frame[0], frame[2], result);
        //동영상 파일에 한 프레임을 저장함.
        writer.write(result);

        imshow("Color", result);

        if (waitKey(1) >= 0)
            break;
    }

    return 0;
}