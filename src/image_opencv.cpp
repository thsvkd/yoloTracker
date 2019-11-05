#ifdef OPENCV

#include "stdio.h"
#include "stdlib.h"
#include "opencv2/opencv.hpp"
#include "image.h"

///////////////////////////////////////////////////////
// this part is edited by Sung Hun //////////////
///////////////////////////////////////////////////////
//#include "ros/ros.h"
//#include "image_transport/image_transport.h"
//#include "opencv2/highgui/highgui.hpp"
//#include "cv_bridge/cv_bridge.h"
////////////////////////////////////////////////////////

using namespace cv;
using namespace std;

extern "C"
{
    //void imageCallback(const sensor_msgs::ImageConstPtr& msg)
    //{
    //    try
    //    {
    //        imshow("view", cv_bridge::toCvShare(msg, "bgr8")->image);
    //        waitKey(30);
    //    }
    //    catch(cv_bridge::exception& e)
    //    {
    //        ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
    //    }
    //}

    IplImage *image_to_ipl(image im)
    {
        int x, y, c;
        IplImage *disp = cvCreateImage(cvSize(im.w, im.h), IPL_DEPTH_8U, im.c);
        int step = disp->widthStep;
        for (y = 0; y < im.h; ++y)
        {
            for (x = 0; x < im.w; ++x)
            {
                for (c = 0; c < im.c; ++c)
                {
                    float val = im.data[c * im.h * im.w + y * im.w + x];
                    disp->imageData[y * step + x * im.c + c] = (unsigned char)(val * 255);
                }
            }
        }
        return disp;
    }

    image ipl_to_image(IplImage *src)
    {
        int h = src->height;
        int w = src->width;
        int c = src->nChannels;
        image im = make_image(w, h, c);
        unsigned char *data = (unsigned char *)src->imageData;
        int step = src->widthStep;
        int i, j, k;

        for (i = 0; i < h; ++i)
        {
            for (k = 0; k < c; ++k)
            {
                for (j = 0; j < w; ++j)
                {
                    im.data[k * w * h + i * w + j] = data[i * step + j * c + k] / 255.;
                }
            }
        }
        return im;
    }

    Mat image_to_mat(image im)
    {
        image copy = copy_image(im);
        constrain_image(copy);
        if (im.c == 3)
            rgbgr_image(copy);

        IplImage *ipl = image_to_ipl(copy);
        Mat m = cvarrToMat(ipl, true);
        cvReleaseImage(&ipl);
        free_image(copy);
        return m;
    }

    image mat_to_image(Mat m)
    {
        IplImage ipl = m;
        image im = ipl_to_image(&ipl);
        rgbgr_image(im);
        return im;
    }

    void *open_video_stream(const char *f, int c, int w, int h, int fps)
    {
        VideoCapture *cap;
        if (f)
            cap = new VideoCapture(f);
        else
            cap = new VideoCapture(c);
        if (!cap->isOpened())
            return 0;
        if (w)
            cap->set(CV_CAP_PROP_FRAME_WIDTH, w);
        if (h)
            cap->set(CV_CAP_PROP_FRAME_HEIGHT, h);
        if (fps)
            cap->set(CV_CAP_PROP_FPS, w);
        return (void *)cap;
    }

    image get_image_from_stream(void *p)
    {
        VideoCapture *cap = (VideoCapture *)p;
        Mat m;
        cout << m.cols;
        *cap >> m;
        if (m.empty())
            return make_empty_image(0, 0, 0);
        return mat_to_image(m);
    }

    image get_image_from_stream_cus(void **p) //////
    {
        VideoCapture **cap = (VideoCapture **)p;
        int i;
        Mat m[CAM_NUM];
        Mat M[2];
        Mat result;

        for (i = 0; i < CAM_NUM; i++)
            *cap[i] >> m[i];
        if (CAM_NUM == 2)
        {
            hconcat(m[0], m[1], result);
        }
        else if (CAM_NUM == 3)
        {
            hconcat(m[0], m[1], M[0]);
            hconcat(M[0], m[2], result);
        }
        else if (CAM_NUM == 4)
        {
            hconcat(m[0], m[1], M[0]);
            hconcat(m[2], m[3], M[1]);
            vconcat(M[0], M[1], result);
        }
        else if (CAM_NUM == 1)
        {
            result = m[0];
        }

        if (result.empty())
            return make_empty_image(0, 0, 0);
        return mat_to_image(result);
    }

    image load_image_cv(char *filename, int channels)
    {
        int flag = -1;
        if (channels == 0)
            flag = -1;
        else if (channels == 1)
            flag = 0;
        else if (channels == 3)
            flag = 1;
        else
        {
            fprintf(stderr, "OpenCV can't force load with %d channels\n", channels);
        }
        Mat m;
        m = imread(filename, flag);
        if (!m.data)
        {
            fprintf(stderr, "Cannot load image \"%s\"\n", filename);
            char buff[256];
            sprintf(buff, "echo %s >> bad.list", filename);
            system(buff);
            return make_image(10, 10, 3);
            //exit(0);
        }
        image im = mat_to_image(m);
        return im;
    }

    int show_image_cv(image im, const char *name, int ms)
    {
        Mat m = image_to_mat(im);
        imshow(name, m);
        int c = waitKey(ms);
        if (c != -1)
            c = c % 256;
        return c;
    }

    void make_window(char *name, int w, int h, int fullscreen)
    {
        namedWindow(name, WINDOW_NORMAL);
        if (fullscreen)
        {
            setWindowProperty(name, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
        }
        else
        {
            resizeWindow(name, w, h);
            if (strcmp(name, "Demo") == 0)
                moveWindow(name, 0, 0);
        }
    }

    void get_video_socket(int *sokt, char serverIP[][20], int *serverPort)
    {
        struct sockaddr_in serverAddr[STREAM];
        socklen_t addrLen[STREAM];

        printf("a\n");

        for (int i = 0; i < STREAM; i++)
        {
            printf("%d socket init start\n", i);
            addrLen[i] = sizeof(struct sockaddr_in);
            if ((sokt[i] = socket(PF_INET, SOCK_STREAM, 0)) < 0)
            {
                printf("socket() failed");
            }

            printf("%d socket() fin\n", i);

            serverAddr[i].sin_family = PF_INET;
            printf("%d serverAddr.sin_family fin\n", i);
            printf("%s\n", serverIP[i]);
            serverAddr[i].sin_addr.s_addr = inet_addr(serverIP[i]);
            printf("%d serverAddr.sin_addr.s_addr fin\n", i);
            serverAddr[i].sin_port = htons(serverPort[i]);

            printf("%d serverAddr() fin\n", i);

            if (connect(sokt[i], (sockaddr *)&serverAddr[i], addrLen[i]) < 0)
            {
                printf("connect() failed!");
            }

            printf("%d socket all fin\n", i);
        }
    }

    image open_video_stream_cus(int *sokt)
    {
        Mat result;
        Mat img[STREAM];

        for (int i = 0; i < STREAM; i++)
        {
            img[i] = Mat::zeros(480, 640, CV_8UC3);
            int imgSize = img[i].total() * img[i].elemSize();
            uchar *iptr = img[i].data;
            int bytes = 0;

            std::cout << "Image Size:" << imgSize << std::endl;

            if ((bytes = recv(sokt[i], iptr, imgSize, MSG_WAITALL)) == -1)
            {
                std::cerr << "recv failed, received bytes = " << bytes << std::endl;
            }
        }

        if (STREAM == 2)
        {
            hconcat(img[0], img[1], result);
        }
        else if (STREAM == 3)
        {
            hconcat(img[0], img[1], img[0]);
            hconcat(img[0], img[2], result);
        }
        else if (STREAM == 4)
        {
            hconcat(img[0], img[1], img[0]);
            hconcat(img[2], img[3], img[2]);
            vconcat(img[0], img[2], result);
        }
        else if (STREAM == 1)
        {
            result = img[0];
        }

        return mat_to_image(result);
    }

    void set_box_ROI(image im, int *left, int *right, int *top, int *bot)
    {
        int center_x = ((*right) - (*left)) / 2 + (*left);
        int center_y = ((*bot) - (*top)) / 2 + (*top);

        if (CAM_NUM == 4 || STREAM == 4)
        {
            if (center_x < im.w / 2 && center_y < im.h / 2)
            {
                if ((*right) > im.w / 2)
                    (*right) = im.w / 2 - 1;
                if ((*bot) > im.h / 2)
                    (*bot) = im.h / 2 - 1;
            }
            else if (center_x >= im.w / 2 && center_y < im.h / 2)
            {
                if ((*left) < im.w / 2)
                    (*left) = im.w / 2;
                if ((*bot) > im.h / 2)
                    (*bot) = im.h / 2 - 1;
            }
            else if (center_x < im.w / 2 && center_y >= im.h / 2)
            {
                if ((*right) > im.w / 2)
                    (*right) = im.w / 2 - 1;
                if ((*top) < im.h / 2)
                    (*top) = im.h / 2;
            }
            else if (center_x >= im.w / 2 && center_y >= im.h / 2)
            {
                if ((*left) < im.w / 2)
                    (*left) = im.w / 2;
                if ((*top) < im.h / 2)
                    (*top) = im.h / 2;
            }
            else if (CAM_NUM == 3 || STREAM == 3)
            {
                if (center_x < im.w / 3)
                {
                    if ((*right) > im.w / 3)
                        (*right) = im.w / 3 - 1;
                }
                else if (center_x >= im.w / 3 && center_x < im.w / 3 * 2)
                {
                    if ((*left) < im.w / 3)
                        (*left) = im.w / 3;
                    if ((*right) > im.w / 3 * 2)
                        (*right) = im.w / 3 * 2 - 1;
                }
                else if (center_x >= im.w / 3 * 2)
                {
                    if ((*left) < im.w / 3 * 2)
                        (*left) = im.w / 3 * 2;
                }
            }
            else if (CAM_NUM == 2 || STREAM == 2)
            {
                if (center_x < im.w / 2)
                {
                    if ((*right) > im.w / 2)
                        (*right) = im.w / 2 - 1;
                }
                else if (center_x >= im.w / 3 && center_x < im.w / 3 * 2)
                {
                    if ((*left) < im.w / 2)
                        (*left) = im.w / 2;
                }
            }
        }
    }
}

#endif
