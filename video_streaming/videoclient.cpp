#include "opencv2/opencv.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

#define WIDTH 640
#define HEIGHT 480

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
    vector<uchar> buff;
    img = Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
    int imgSize;
    int bytes = 0;
    int key;

    if (!img.isContinuous())
    {
        img = img.clone();
    }

    //std::cout << "Image Size:" << imgSize << std::endl;

    namedWindow("CV Video Client", 1);

    while (key != 'q')
    {

        if ((bytes = recv(sokt, &imgSize, sizeof(imgSize), MSG_WAITALL)) == -1)
        {
            std::cerr << "recv failed, received bytes = " << bytes << std::endl;
        }
        buff.resize(imgSize);

        if ((bytes = recv(sokt, buff.data(), imgSize, MSG_WAITALL)) == -1)
        {
            std::cerr << "recv failed, received bytes = " << bytes << std::endl;
        }

        img = imdecode(Mat(buff), 1);

        cv::imshow("CV Video Client", img);

        if (key = cv::waitKey(10) >= 0)
            break;
    }

    close(sokt);

    return 0;
}