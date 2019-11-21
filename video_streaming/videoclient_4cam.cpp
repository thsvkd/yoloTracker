#include "opencv2/opencv.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

using namespace cv;
using namespace std;

void *display(void *);
int find_index(vector<int> input, int para);

Mat img_p[4];
vector<uchar> rev_img_p[4];
vector<int> sokt = vector<int>(4);
Mat result = Mat::zeros(960, 1280, CV_8UC3);
Mat imageROI[4];

int main(int argc, char **argv)
{

    //--------------------------------------------------------
    //networking stuff: socket , connect
    //--------------------------------------------------------
    char serverIP[] = "114.70.22.21";
    int serverPort[4] = {5001, 5002, 5003, 5004};

    struct sockaddr_in serverAddr[4];
    socklen_t addrLen = sizeof(struct sockaddr_in);

    for (int i = 0; i < 4; i++)
    {
        if ((sokt[i] = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            std::cerr << "socket( " << i << " ) failed" << std::endl;
        }

        serverAddr[i].sin_family = PF_INET;
        serverAddr[i].sin_addr.s_addr = inet_addr(serverIP);
        serverAddr[i].sin_port = htons(serverPort[i]);

        if (connect(sokt[i], (sockaddr *)&serverAddr, addrLen) < 0)
        {
            std::cerr << "connect( " << i << " ) failed!" << std::endl;
        }

        cout << sokt[i] << endl;
    }

    //----------------------------------------------------------
    //OpenCV Code
    //----------------------------------------------------------

    int imgSize;
    int bytes = 0;
    int key;
    pthread_t thread_id[4];

    namedWindow("CV Video Client", 1);
    vector<uchar> rev_img;

    for (int i = 0; i < 4; i++)
        img_p[i] = Mat::zeros(480, 640, CV_8UC3);

    imageROI[0] = Mat(result, Rect(0, 0, 640, 480));
    imageROI[1] = Mat(result, Rect(640, 0, 640, 480));
    imageROI[2] = Mat(result, Rect(0, 480, 640, 480));
    imageROI[3] = Mat(result, Rect(640, 480, 640, 480));

    cout << "while loop start" << endl;

    while (key != 'q')
    {
        for (int i = 0; i < 4; i++)
            pthread_create(&thread_id[i], NULL, display, &sokt[i]);

        for (int i = 0; i < 4; i++)
            pthread_join(thread_id[i], NULL);

        //hconcat(img_p[0], img_p[1], img_p[0]);
        //hconcat(img_p[2], img_p[3], img_p[2]);
        //vconcat(img_p[0], img_p[2], result);

        imshow("CV Video Client", result);

        if (key = waitKey(10) >= 0)
            break;
    }

    for (int i = 0; i < 4; i++)
        close(sokt[i]);

    return 0;
}

void *display(void *ptr)
{
    int socket = *(int *)ptr;
    int bytes = 0;
    int imgSize;
    int index = find_index(sokt, socket);
    cout << index << " pthread started" << endl;

    while (1)
    {
        if ((bytes = recv(socket, &imgSize, sizeof(imgSize), MSG_WAITALL)) == -1)
        {
            cerr << " recv failed, received bytes = " << bytes << endl;
        }
        rev_img_p[index].resize(imgSize);
        cout << index << " imgSize is " << imgSize << endl;

        if ((bytes = recv(socket, rev_img_p[index].data(), imgSize, MSG_WAITALL)) == -1)
        {
            cerr << " recv failed, received bytes = " << bytes << endl;
        }

        img_p[index] = imdecode(Mat(rev_img_p[index]), CV_LOAD_IMAGE_COLOR);
        img_p[index].copyTo(imageROI[index], img_p[index]);
    }
}

int find_index(vector<int> input, int para)
{
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == para)
            return i;
    }

    return -1;
}