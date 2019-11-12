#include "tracker.h"

#define MAXLINE 511 //최대값 지정
#define BLOCK 255   //BLOCK 단위로 저장
#define THING_NUM 20

#define CURRENT 0
#define PREVIOUS 1

using namespace cv;
using namespace std;

int serv_sock;
int clnt_sock;
struct sockaddr_in servaddr;
struct sockaddr_in clntaddr;
int addrlen = sizeof(servaddr); //서버 주소의 size를 저장
socklen_t clnt_addr_size;

vector<tracking_dot> trackers_dot;
quadrant Quadrant;
vector<thing_color> tag_color;

int width, height;

int main(int argc, char **argv)
{
    int s = connect_to_server("192.168.1.7", "8000");
    string msg;
    char buf[512] = {0};

    namedWindow("Tracker", WINDOW_AUTOSIZE);

    vector<vector<string>> file = read_txt("../out.txt");
    Mat frame;
    vector<thing_info> current_thing;
    get_frame_size(file);

    while (1)
    {
        file = read_txt("../out.txt"); // comment
        get_frame_size(file);

        if (file.size() == 0)
            continue;

        if (image_read_enable(file))
        {
            frame = imread("../out.jpg", IMREAD_COLOR);

            if (frame.empty())
                break;

            current_thing = file_to_box(frame, file); //current_thing에는 욜로가 생성한 물체들에대한 정보가 임시로 저장 되어 있음

            putText(frame, file[0][5], Point(30, 30), 2, 1, Scalar(255, 255, 0));

            cout << "update_dot start" << endl;
            for (int i = 0; i < trackers_dot.size(); i++) //tracking thing
            {
                cout << i << "trackers_dot update start" << endl;
                int flag = trackers_dot[i].update_dot(frame, current_thing);
                if (trackers_dot[i].bbox.h > trackers_dot[i].bbox.w)
                    trackers_dot[i].distance_limit = trackers_dot[i].bbox.h * height / 2;
                else
                    trackers_dot[i].distance_limit = trackers_dot[i].bbox.w * width / 2;

                if (flag != -1)
                    current_thing[flag].hit = 1;
                //trackers_dot[i].miss_stack > trackers_dot[i].miss_limit
                if (trackers_dot[i].is_missed)
                {
                    if (trackers_dot[i].if_tracker_get_out_screen() || trackers_dot[i].miss_stack > trackers_dot[i].miss_limit)
                    {
                        thing_color tmp;
                        tmp.tag = trackers_dot[i].tag;
                        tmp.box_color = trackers_dot[i].box_color;
                        tag_color.push_back(tmp);
                        trackers_dot.erase(trackers_dot.begin() + i);
                        i--;
                        cout << i << " tracker_dot deleted" << endl;
                    }
                }
            }
            cout << "update_dot end" << endl;

            cout << "trackers_dot.push_back() start" << endl;
            for (int i = 0; i < current_thing.size(); i++)
            {
                if (current_thing[i].hit == 0) //new thing
                {
                    tracking_dot tmp;
                    tmp.bbox = current_thing[i].bbox;
                    if (tmp.bbox.h > tmp.bbox.w)
                        tmp.distance_limit = tmp.bbox.h * height / 2;
                    else
                        tmp.distance_limit = tmp.bbox.w * width / 2;
                    tmp.stack_point = vector<Point>(60, Point(-1, -1));
                    tmp.im = current_thing[i].im;
                    tmp.p = cal_center_point(current_thing[i].bbox);
                    tmp.name = current_thing[i].name;
                    tmp.velocity = Point(0, 0);
                    tmp.tag = get_empty_tag();
                    tmp.is_missed = false;
                    tmp.what_quadrant_am_i();
                    tmp.box_color = make_random_color();

                    for (int i = 0; i < Quadrant.pos.size(); i++)
                    {
                        if (tmp.position == Quadrant.pos[i])
                        {
                            vector<int> tmp2 = Quadrant.still_thing(i);
                            tmp.tag = tmp2[1];
                            break;
                        }
                    }

                    for (int i = 0; i < tag_color.size(); i++)
                    {
                        if (tag_color[i].tag = trackers_dot[i].tag)
                            tmp.box_color = tag_color[i].box_color;
                    }

                    trackers_dot.push_back(tmp);
                    trackers_dot[trackers_dot.size() - 1].put_point_to_stack(tmp.p);
                    current_thing[i].hit = 1;
                }
            }
            cout << "trackers_dot.push_back() end" << endl;

            cout << "draw box start" << endl;
            for (int i = 0; i < trackers_dot.size(); i++)
            {
                if (!trackers_dot[i].is_missed)
                {
                    rectangle(frame, box_to_Rect2d(trackers_dot[i].bbox), trackers_dot[i].box_color, 2, 1);
                    //circle(frame, trackers_dot[i].p, 2, Scalar(255, 0, 0), -1);                             //for debug
                    //circle(frame, trackers_dot[i].p, trackers_dot[i].distance_limit, Scalar(255, 0, 0), 2); //for debug
                    //circle(frame, trackers_dot[i].predict_next_point(), 2, Scalar(0, 0, 255), -1);          //for debug
                    putText(frame,
                            to_string(trackers_dot[i].tag),
                            Point(trackers_dot[i].bbox.x * width, trackers_dot[i].bbox.y * height),
                            2,
                            1,
                            trackers_dot[i].box_color);
                }
                // else
                // {
                //     rectangle(frame, box_to_Rect2d(trackers_dot[i].bbox), Scalar(0, 255, 0), 2, 1);
                //     circle(frame, trackers_dot[i].p, 2, Scalar(0, 255, 0), -1);                             //for debug
                //     circle(frame, trackers_dot[i].p, trackers_dot[i].distance_limit, Scalar(0, 255, 0), 2); //for debug
                //     circle(frame, trackers_dot[i].predict_next_point(), 2, Scalar(0, 0, 255), -1);          //for debug
                //     putText(frame,
                //             to_string(trackers_dot[i].tag) + "miss stack = " + to_string(trackers_dot[i].miss_stack),
                //             Point(trackers_dot[i].bbox.x * width, trackers_dot[i].bbox.y * height),
                //             2,
                //             1,
                //             Scalar(0, 255, 0));
                // }
            }
            cout << "draw box start" << endl;
            msg = make_msg(); //이 함수 나중에 고쳐야함!!!!!! comment
            sendMessage(s, msg.c_str());
            msg = "";
            make_txt(file);

            imwrite("debug_img/debug_img_" + string(file[0][5]) + ".jpg", frame);
        }

        imshow("Tracker", frame);
        if (waitKey(1) == 'q')
            break;
    }
}

Point cal_center_point(box bbox)
{
    int x_center = (bbox.x + bbox.w / 2) * width;
    int y_center = (bbox.y + bbox.h / 2) * height;

    return Point(x_center, y_center);
}

void init_dot(int size)
{
    tracking_dot tmp;
    tmp.tracker = TrackerMOSSE::create();
    tmp.stack_point = vector<Point>(10, Point(-1, -1));
    tmp.bbox = Rect2d_to_box(Rect2d(-1, -1, -1, -1));
    tmp.p = Point(-1, -1);
    tmp.name = "";
    tmp.velocity = Point(-1, -1);
    tmp.tag = -1;
    tmp.is_missed = true;

    for (int i = 0; i < size; i++)
        trackers_dot.push_back(tmp);
}

void put_init_value_to_dot(vector<thing_info> current_thing)
{
    for (int i = 0; i < current_thing.size(); i++)
    {
        trackers_dot[i].bbox = current_thing[i].bbox;
        trackers_dot[i].p = cal_center_point(current_thing[i].bbox);
        trackers_dot[i].name = current_thing[i].name;
        trackers_dot[i].tag = i;
        trackers_dot[i].im = current_thing[i].im;
    }
}

void init_msg(char *msg)
{
    for (int i = 0; i < strlen(msg); i++)
        msg[i] = 0;
}

string make_msg() //수정 필요!
{
    string tmp;

    for (int i = 0; i < trackers_dot.size(); i++)
    {
        if (!trackers_dot[i].is_missed)
        {
            tmp += trackers_dot[i].name + " " +
                   to_string(trackers_dot[i].bbox.x) + " " +
                   to_string(trackers_dot[i].bbox.y) + " " +
                   to_string(trackers_dot[i].bbox.w) + " " +
                   to_string(trackers_dot[i].bbox.h) + " " +
                   to_string(trackers_dot[i].tag) + "\n";
        }
    }

    return tmp;
}

void make_txt(vector<vector<string>> file)
{
    ofstream p("../out.txt");

    for (int i = 0; i < file.size(); i++)
    {
        if (i == 0)
            p << file[i][0] << " " << file[i][1] << " " << file[i][2] << " " << file[i][3] << " "
              << "0" << endl;
        else if (i != file.size() - 1)
            p << file[i][0] << " " << file[i][1] << " " << file[i][2] << " " << file[i][3] << endl;
        else
            p << file[i][0] << " " << file[i][1] << " " << file[i][2] << " " << file[i][3];
    }
}

int image_read_enable(vector<vector<string>> file)
{
    return !file[0][4].compare("1") ? 1 : 0;
}

void get_frame_size(vector<vector<string>> file)
{
    if (file.size() == 0)
        return;
    else
    {
        if (file[0].size() == 0)
            return;
    }

    stringstream ss(file[0][0]);
    vector<int> int_tmp;
    int i = 0;
    while (ss.good())
    {
        string tmp;
        getline(ss, tmp, 'x');
        int_tmp.push_back(atoi(tmp.c_str()));
    }
    width = int_tmp[0];
    height = int_tmp[1];
}

void sendMessage(int s, const char *buf)
{
    if ((sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, addrlen)) < 0)
    {
        perror("sendto fail");
        exit(0);
    }
}

int connect_to_server(char *ip, char *port)
{
    int s;

    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket fail");
        exit(0);
    }

    memset(&servaddr, 0, addrlen);            //bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;            //인터넷 Addr Family
    servaddr.sin_addr.s_addr = inet_addr(ip); //argv[1]에서 주소를 가져옴
    servaddr.sin_port = htons(atoi(port));    //argv[2]에서 port를 가져옴

    return s;
}

vector<vector<string>> read_txt(string file_path)
{
    vector<vector<string>> result;
    ifstream p(file_path);
    if (!p.is_open())
        return result;

    stringstream ss;

    while (!p.eof())
    {
        string tmp;
        getline(p, tmp, '\n');

        if (tmp.size() == 0)
            break;
        stringstream ss(tmp);
        vector<string> file;

        while (ss.good())
        {
            getline(ss, tmp, ' ');
            file.push_back(tmp);
        }

        result.push_back(file);
    }

    return result;
}

Rect2d box_to_Rect2d(box bbox)
{
    Rect2d tmp;
    tmp.x = (int)(bbox.x * width);
    tmp.y = (int)(bbox.y * height);
    tmp.width = (int)(bbox.w * width);
    tmp.height = (int)(bbox.h * height);

    return tmp;
}

box Rect2d_to_box(Rect2d rect2d)
{
    box tmp;
    tmp.x = (float)rect2d.x / (float)width;
    tmp.y = (float)rect2d.y / (float)height;
    tmp.w = (float)rect2d.width / (float)width;
    tmp.h = (float)rect2d.height / (float)height;

    return tmp;
}

vector<thing_info> file_to_box(Mat im, vector<vector<string>> file)
{
    vector<thing_info> result;
    for (int i = 1; i < file.size(); i++)
    {
        thing_info tmp;

        tmp.name = file[i][0];
        tmp.bbox.x = atof(file[i][1].c_str());
        tmp.bbox.y = atof(file[i][2].c_str());
        tmp.bbox.w = atof(file[i][3].c_str());
        tmp.bbox.h = atof(file[i][4].c_str());
        tmp.im = im(Rect2d(tmp.bbox.x * width, tmp.bbox.y * height, tmp.bbox.w * width, tmp.bbox.h * height));
        tmp.tag = -1;
        tmp.hit = 0;

        result.push_back(tmp);
    }

    return result;
}

int get_empty_tag()
{
    srand((unsigned int)clock());
    int tag = rand() % 1000;
    int make_tag_success = 1;

    while (make_tag_success == 0)
    {
        tag = rand() % 1000;
        make_tag_success = 1;

        for (int i = 0; i < trackers_dot.size(); i++)
        {
            if (tag == trackers_dot[i].tag)
            {
                make_tag_success = 0;
                break;
            }
        }
    }

    return tag;
}

// Rect2d mosse_tracker_update(Mat frame, int tag)
// {
//     Rect2d bbox;
//     if (tag != -1)
//     {
//         int index = (thing_buf_index % 2) * THING_NUM + tag;
//         float x = things[index].bbox.x * width;
//         float y = things[index].bbox.y * height;
//         float w = things[index].bbox.w * width;
//         float h = things[index].bbox.h * height;
//         bbox = Rect2d((int)x, (int)y, (int)w, (int)h);

//         trackers[index] = TrackerMOSSE::create();
//         bool t_ok = trackers[tag]->init(frame, bbox);
//     }

//     return bbox;
// }

float cal_distance(Point input1, thing_info input2)
{
    float distance = 0.0;
    Point p2 = cal_center_point(input2.bbox);
    float x_line = input1.x - p2.x;
    float y_line = input1.y - p2.y;

    distance = sqrt(pow(x_line, 2) + pow(y_line, 2));

    return distance;
}

vector<int> get_least_dis_index_list(vector<float> dis_list, float limit)
{
    vector<int> result;

    for (int i = 0; i < dis_list.size(); i++)
    {
        if (dis_list[i] < limit && (dis_list[i] >= 0))
            result.push_back(i);
    }

    return result;
}

Scalar make_random_color()
{
    srand((unsigned int)clock());
    int i = 0;
    int r = rand() % 256;
    while (i++ != 10)
    {
    }
    i = 0;
    int g = rand() % 256;
    while (i++ != 10)
    {
    }
    int b = rand() % 256;

    return Scalar(r, g, b);
}

Scalar get_thing_color()
{
}

// vector<Rect2d> watchdog(Mat frame, vector<thing_info> current_thing)
// {
//     float r_limit = 5.0;
//     vector<Rect2d> result;
//     Rect2d tmp_2d;

//     for (int i = 0; i < THING_NUM; i++)
//         trackers_dot[i].is_missed = true;

//     for (int i = 0; i < current_thing.size(); i++)
//     {
//         int flag = thing_exist(current_thing[i]);
//         if (flag >= 0) //기존 물체라면 현배 버퍼의 같은 위치로 옮겨온다.
//         {
//             trackers_dot[flag].im = current_thing[i].im;
//             trackers_dot[flag].bbox = current_thing[i].bbox;
//             trackers_dot[flag].name = current_thing[i].name;
//             trackers_dot[flag].p = cal_center_point(current_thing[i].bbox);
//             trackers_dot[flag].put_point_to_stack(trackers_dot[flag].p);
//             trackers_dot[flag].is_missed = false;
//             trackers_dot[flag].miss_stack = 0;
//         }
//         else if (flag == -1)
//         {
//             int new_tag = get_empty_tag();
//             trackers_dot[new_tag].im = current_thing[i].im;
//             trackers_dot[new_tag].bbox = current_thing[i].bbox;
//             trackers_dot[new_tag].name = current_thing[i].name; //만약 새 물체라면 현재 버퍼의 새 위치로 옮겨온다.
//             trackers_dot[new_tag].p = cal_center_point(current_thing[i].bbox);
//             trackers_dot[new_tag].put_point_to_stack(trackers_dot[new_tag].p);
//             trackers_dot[new_tag].is_missed = false;
//             trackers_dot[new_tag].miss_stack = 0;
//         }
//     }

//     for (int i = 0; i < THING_NUM; i++) //사물들의 박스를 그리기위한 좌표를 생성
//     {
//         if (trackers_dot[i].tag != -1)
//         {
//             if (trackers_dot[i].is_missed == false)
//             {
//                 tmp_2d = box_to_Rect2d(trackers_dot[i].bbox);
//                 result.push_back(tmp_2d);
//             }
//             else
//             {
//                 trackers_dot[i].goto_next_point();
//                 trackers_dot[i].is_missed++;

//                 if (trackers_dot[i].miss_stack > 10)
//                     trackers_dot[i].tag = -1;
//             }
//         }
//     }

//     return result;
// }
