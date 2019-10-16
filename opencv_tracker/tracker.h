#ifndef TRACKER_H
#define TRACKER_H

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/features2d.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

typedef struct
{
    float x, y, w, h;
} box;

typedef struct thing_info
{
    Mat im;
    box bbox;
    int tag;
    string name;
    int hit;
} thing_info;

class tracking_dot
{
private:
public:
    Ptr<Tracker> tracker;
    vector<Point> stack_point;
    Mat im;
    box bbox;
    Point p;
    string name;
    Point velocity;
    int tag;
    int stack_num = 10;
    int miss_stack = 0;
    bool is_missed = false;

    tracking_dot();
    tracking_dot(Point pp);
    tracking_dot(Point pp, int tagg);
    Point get_point();
    int get_tag();
    Point get_v();
    void put_point_to_stack(Point pp);
    Point cal_v();
    bool is_empty();
    void goto_next_point();
    Point predict_next_point();
    float cal_distance_score(vector<thing_info> current_thing, int distance_limit);
    float cal_matching_score(vector<thing_info> current_thing, int score_limit);
    int get_dot_thing(vector<thing_info> current_thing);
};

float doublecalcAngleFromPoints(Point2f _ptFirstPos, Point2f _ptSecondPos);
void alignImages(Mat &im1, Mat &im2, Mat &im1Reg, Mat &h);
float align_Images(Mat &im1, Mat &im2);

Point cal_center_point(box bbox);
void init_dot(int size);
void put_init_value_to_dot(vector<thing_info> tmp_thing);
void init_msg(char *msg);
string make_msg(vector<vector<string>> file);
void make_txt(vector<vector<string>> file);
int image_read_enable(vector<vector<string>> file);
void get_frame_size(vector<vector<string>> file);
void sendMessage(int s, const char *buf);
int connect_to_server(char *ip, char *port);
vector<vector<string>> read_txt(string file_path);
Rect2d box_to_Rect2d(box bbox);
box Rect2d_to_box(Rect2d rect2d);
vector<thing_info> file_to_box(Mat im, vector<vector<string>> file);
void init_mosse_tracker();
void init_thing_info();
void init_buf_info();
void init_hit();
int get_empty_tag();
Rect2d mosse_tracker_update(Mat frame, int tag);
Rect2d mosse_tracker_show(Mat frame, int tag);
int get_MAX_index_of_things();
float cal_distance(tracking_dot *input1, thing_info input2);
vector<int> get_least_dis_index_list(vector<float> dis_list, float limit);
int thing_exist(thing_info input);
vector<Rect2d> watchdog(Mat frame, vector<thing_info> current_thing);

#endif
