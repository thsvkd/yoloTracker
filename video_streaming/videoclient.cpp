#include "opencv2/opencv.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
<<<<<<< HEAD
=======

#define WIDTH 1280
#define HEIGHT 960
>>>>>>> 4030b4e4aaf165a59a0bb7a2f51ae1e201bcda58

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
    int sokt;
    char *serverIP;
    int serverPort;

    if (argc < 3)
    {
        std::cerr << "Usage: cv_video_cli <serverIP> <serverPort> " << std::endl;
    }

    serverIP = argv[1];
    serverPort = atoi(argv[2]);

    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(struct sockaddr_in);

    if ((sokt = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "socket() failed" << std::endl;
    }

    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(serverPort);

    if (connect(sokt, (sockaddr *)&serverAddr, addrLen) < 0)
    {
        std::cerr << "connect() failed!" << std::endl;
    }

    Mat img;
<<<<<<< HEAD
    img = Mat::zeros(480, 640, CV_8UC3);
    int imgSize;
    uchar *iptr = img.data;
=======
    vector<uchar> buff;
    //img = Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
    int imgSize;
>>>>>>> 4030b4e4aaf165a59a0bb7a2f51ae1e201bcda58
    int bytes = 0;
    int key;

    if (!img.isContinuous())
    {
        img = img.clone();
    }

    //std::cout << "Image Size:" << imgSize << std::endl;

    namedWindow("CV Video Client", 1);
    vector<uchar> rev_img;

    while (key != 'q')
    {
<<<<<<< HEAD
        if ((bytes = recv(sokt, &imgSize, sizeof(imgSize), MSG_WAITALL)) == -1)
=======

        if ((bytes = recv(sokt, &imgSize, sizeof(imgSize), MSG_WAITALL)) == -1)
        {
            std::cerr << "recv failed, received bytes = " << bytes << std::endl;
        }
        buff.resize(imgSize);

        if ((bytes = recv(sokt, buff.data(), imgSize, MSG_WAITALL)) == -1)
>>>>>>> 4030b4e4aaf165a59a0bb7a2f51ae1e201bcda58
        {
            std::cerr << "recv failed, received bytes = " << bytes << std::endl;
        }
        rev_img.resize(imgSize);

<<<<<<< HEAD
        if ((bytes = recv(sokt, rev_img.data(), imgSize, MSG_WAITALL)) == -1)
        {
            std::cerr << "recv failed, received bytes = " << bytes << std::endl;
        }
        img = imdecode(Mat(rev_img), CV_LOAD_IMAGE_COLOR);
=======
        img = imdecode(Mat(buff), 1);

>>>>>>> 4030b4e4aaf165a59a0bb7a2f51ae1e201bcda58
        cv::imshow("CV Video Client", img);

        if (key = cv::waitKey(10) >= 0)
            break;
    }

    close(sokt);

    return 0;
}