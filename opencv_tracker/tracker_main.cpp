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

int thing_buf_index;
vector<thing_info> things(THING_NUM * 2);
Ptr<Tracker> trackers[THING_NUM];
vector<tracking_dot> trackers_dot;
vector<int> tag_table;

int width, height;

int main(int argc, char **argv)
{
    Rect2d bbox_tmp;
    thing_buf_index = 0;
    // int s = connect_to_server("192.168.43.95", "8000");
    string msg;
    char buf[512] = {0};

    namedWindow("Tracker", WINDOW_AUTOSIZE);

    vector<vector<string>> file = read_txt("../out.txt");
    Mat frame;
    vector<thing_info> current_thing;
    get_frame_size(file);

    while (1)
    {
        file = read_txt("../out.txt");
        get_frame_size(file);

        if (file.size() == 0)
            continue;

        if (image_read_enable(file))
        {
            frame = imread("../out.jpg", IMREAD_COLOR);
            vector<Rect2d> bbox_tmp;

            if (frame.empty())
                break;

            current_thing = file_to_box(frame, file); //current_thing에는 욜로가 생성한 물체들에대한 정보가 임시로 저장 되어 있음

            //bbox_tmp = watchdog(frame, current_thing);

            for (int i = 0; i < trackers_dot.size(); i++) //tracking thing
            {
                trackers_dot[i].update_dot(current_thing);
            }

            for (int i = 0; i < current_thing.size(); i++)
            {
                if (current_thing[i].hit == 0) //new thing
                {
                    tracking_dot tmp;
                    tmp.tracker = TrackerMOSSE::create();
                    tmp.stack_point = vector<Point>(10, Point(-1, -1));
                    tmp.im = current_thing[i].im;
                    tmp.bbox = current_thing[i].bbox;
                    tmp.p = cal_center_point(current_thing[i].bbox);
                    tmp.name = current_thing[i].name;
                    tmp.velocity = Point(0, 0);
                    tmp.tag = get_empty_tag();
                    tmp.is_missed = false;
                    trackers_dot.push_back(tmp);
                    trackers_dot[trackers_dot.size() - 1].put_point_to_stack(tmp.p);
                    current_thing[i].hit = 1;
                }
            }

            for (int i = 0; i < trackers_dot.size(); i++) //delete missed thing
            {
                if (trackers_dot[i].tag == -1)
                    trackers_dot.erase(trackers_dot.begin() + i);
            }

            for (int i = 0; i < trackers_dot.size(); i++)
            {
                rectangle(frame, box_to_Rect2d(trackers_dot[i].bbox), Scalar(255, 0, 0), 2, 1);
                circle(frame, trackers_dot[i].p, 2, Scalar(255, 0, 0), -1);                    //for debug
                circle(frame, trackers_dot[i].p, 50, Scalar(255, 0, 0), 2);                    //for debug
                circle(frame, trackers_dot[i].predict_next_point(), 2, Scalar(0, 0, 255), -1); //for debug
                putText(frame, to_string(trackers_dot[i].tag), Point(trackers_dot[i].bbox.x * width, trackers_dot[i].bbox.y * height), 2, 2, Scalar(255, 0, 0));
            }
            //msg = make_msg(file);     //이 함수 나중에 고쳐야함!!!!!!
            //sendMessage(s, msg.c_str());
            msg = "";
            make_txt(file);
        }

        // if (waitKey(1) == 'q')
        //     break;

        imshow("Tracker", frame);
        thing_buf_index++;
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

string make_msg(vector<vector<string>> file) //수정 필요!
{
    string tmp;

    for (int i = 1; i < file.size(); i++)
    {
        int index = (thing_buf_index % 2) * THING_NUM + i - 1;
        tmp += file[i][0] + " " + file[i][1] + " " + file[i][2] + " " + file[i][3] + " " + file[i][4] + " ";
        tmp += to_string(trackers_dot[index].tag);
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
        int index = (thing_buf_index % 2) * THING_NUM + i - 1;

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

void init_mosse_tracker()
{
    for (int i = 0; i < THING_NUM; i++)
        trackers[i] = TrackerMOSSE::create();
}

void init_thing_info()
{
    box bbox = {-1, -1, -1, -1};

    for (int i = 0; i < THING_NUM; i++)
    {
        //int index = (thing_buf_index % 2) * THING_NUM + i;
        things[0 * THING_NUM + i].tag = -1;
        things[0 * THING_NUM + i].name = "";
        things[0 * THING_NUM + i].bbox = bbox;
        things[0 * THING_NUM + i].hit = 0;
        things[1 * THING_NUM + i].tag = -1;
        things[1 * THING_NUM + i].bbox = bbox;
        things[1 * THING_NUM + i].name = "";
        things[1 * THING_NUM + i].hit = 0;
    }
}

void init_buf_info()
{
    box bbox = {-1, -1, -1, -1};
    int index_c_base = (thing_buf_index % 2) * THING_NUM;
    int index_p_base = (thing_buf_index % 2 + 1) * THING_NUM;

    for (int i = index_c_base; i < index_c_base + THING_NUM; i++)
    {
        things[i].tag = -1;
        things[i].bbox = bbox;
        things[i].name = "";
        things[i].hit = 0;
    }
}

void init_hit()
{
    for (int i = 0; i < THING_NUM; i++)
    {
        things[0 * THING_NUM + i].hit = 0;
        things[1 * THING_NUM + i].hit = 0;
    }
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

Rect2d mosse_tracker_update(Mat frame, int tag)
{
    Rect2d bbox;
    if (tag != -1)
    {
        int index = (thing_buf_index % 2) * THING_NUM + tag;
        float x = things[index].bbox.x * width;
        float y = things[index].bbox.y * height;
        float w = things[index].bbox.w * width;
        float h = things[index].bbox.h * height;
        bbox = Rect2d((int)x, (int)y, (int)w, (int)h);

        trackers[index] = TrackerMOSSE::create();
        bool t_ok = trackers[tag]->init(frame, bbox);
    }

    return bbox;
}

Rect2d mosse_tracker_show(Mat frame, int tag)
{
    box bbbox;
    Rect2d bbbox_d;

    if (tag != -1)
    {
        int index = (thing_buf_index % 2) * THING_NUM + tag;
        bbbox.x = things[index].bbox.x;
        bbbox.y = things[index].bbox.y;
        bbbox.w = things[index].bbox.w;
        bbbox.h = things[index].bbox.h;

        bbbox_d = Rect2d((int)(bbbox.x * width), (int)(bbbox.y * height), (int)(bbbox.w * width), (int)(bbbox.h * height));
        trackers[tag]->update(frame, bbbox_d);

        things[index].bbox.x = (float)bbbox_d.x / (float)width;
        things[index].bbox.y = (float)bbbox_d.y / (float)height;
        things[index].bbox.w = (float)bbbox_d.width / (float)width;
        things[index].bbox.h = (float)bbbox_d.height / (float)height;
    }

    return bbbox_d;
}

int get_MAX_index_of_things()
{
    for (int i = THING_NUM - 1; i >= 0; i--)
    {
        if (things[i].tag != -1)
            return i;
    }
}

float cal_distance(tracking_dot *input1, thing_info input2)
{
    float distance = 0.0;
    float limit = 50;
    Point p2 = cal_center_point(input2.bbox);
    float x_line = input1->p.x - p2.x;
    float y_line = input1->p.y - p2.y;

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

int thing_exist(thing_info input) //존재하면 존재한 사물의 인덱스를 반환 존재 하지 않으면 -1을 반환
{
    float score = -1;
    float score_limit = 2.5;
    float distance_limit = 50;
    vector<float> dis_list;
    vector<int> index_list;

    for (int i = 0; i < THING_NUM; i++)
    {
        if (trackers_dot[i].tag != -1)
            dis_list.push_back(cal_distance(&trackers_dot[i], input));
    }

    index_list = get_least_dis_index_list(dis_list, distance_limit);
    sort(index_list.begin(), index_list.end());

    if (index_list.size() == 1)
        return index_list[0];
    else if (index_list.size() > 1)
    {
        for (int i = 0; i < index_list.size(); i++)
        {
            if (!trackers_dot[index_list[i]].im.empty())
            {
                cout << "align_Images start" << endl;
                cout << trackers_dot[index_list[i]].im.cols << " " << input.im.cols << endl;
                score = align_Images(trackers_dot[index_list[i]].im, input.im);
            }

            if (score < score_limit && (score >= 0))
                return index_list[i];
        }
    }

    return -1;
}

vector<Rect2d> watchdog(Mat frame, vector<thing_info> current_thing)
{
    float r_limit = 5.0;
    vector<Rect2d> result;
    Rect2d tmp_2d;

    for (int i = 0; i < THING_NUM; i++)
        trackers_dot[i].is_missed = true;

    for (int i = 0; i < current_thing.size(); i++)
    {
        int flag = thing_exist(current_thing[i]);
        if (flag >= 0) //기존 물체라면 현배 버퍼의 같은 위치로 옮겨온다.
        {
            trackers_dot[flag].im = current_thing[i].im;
            trackers_dot[flag].bbox = current_thing[i].bbox;
            trackers_dot[flag].name = current_thing[i].name;
            trackers_dot[flag].p = cal_center_point(current_thing[i].bbox);
            trackers_dot[flag].put_point_to_stack(trackers_dot[flag].p);
            trackers_dot[flag].is_missed = false;
            trackers_dot[flag].miss_stack = 0;
        }
        else if (flag == -1)
        {
            int new_tag = get_empty_tag();
            trackers_dot[new_tag].im = current_thing[i].im;
            trackers_dot[new_tag].bbox = current_thing[i].bbox;
            trackers_dot[new_tag].name = current_thing[i].name; //만약 새 물체라면 현재 버퍼의 새 위치로 옮겨온다.
            trackers_dot[new_tag].p = cal_center_point(current_thing[i].bbox);
            trackers_dot[new_tag].put_point_to_stack(trackers_dot[new_tag].p);
            trackers_dot[new_tag].is_missed = false;
            trackers_dot[new_tag].miss_stack = 0;
        }
    }

    for (int i = 0; i < THING_NUM; i++) //사물들의 박스를 그리기위한 좌표를 생성
    {
        if (trackers_dot[i].tag != -1)
        {
            if (trackers_dot[i].is_missed == false)
            {
                tmp_2d = box_to_Rect2d(trackers_dot[i].bbox);
                result.push_back(tmp_2d);
            }
            else
            {
                trackers_dot[i].goto_next_point();
                trackers_dot[i].is_missed++;

                if (trackers_dot[i].miss_stack > 10)
                    trackers_dot[i].tag = -1;
            }
        }
    }

    return result;
}
