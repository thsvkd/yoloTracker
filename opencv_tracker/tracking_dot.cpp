#include "tracker.h"

using namespace std;
using namespace cv;

tracking_dot::tracking_dot()
{
    p = Point(0, 0);
    tag = -1;
    velocity = Point(-1, -1);
    stack_point = vector<Point>(stack_num);
}

tracking_dot::tracking_dot(Point pp)
{
    p.x = pp.x;
    p.y = pp.y;
    tag = -1;
    velocity = Point(-1, -1);
    stack_point = vector<Point>(stack_num);
}

tracking_dot::tracking_dot(Point pp, int tagg)
{
    p.x = pp.x;
    p.y = pp.y;
    tag = tagg;
    velocity = Point(-1, -1);
    stack_point = vector<Point>(stack_num);
}

Point tracking_dot::get_point()
{
    return p;
}

int tracking_dot::get_tag()
{
    return tag;
}

Point tracking_dot::get_v()
{
    velocity = cal_v();
    return velocity;
}

void tracking_dot::put_point_to_stack(Point pp)
{
    for (int i = stack_num - 1; i > 0; i--)
        stack_point[i] = stack_point[i - 1];

    stack_point[0] = pp;
}

Point tracking_dot::cal_v()
{
    float sum_x = 0, sum_y = 0;
    int stack_num = 0;

    for (int i = 0; i < stack_point.size() - 1; i++)
        if (stack_point[i].x != -1)
            stack_num++;

    sum_x = (float)(stack_point[stack_num - 1].x - stack_point[0].x) / (float)stack_num;
    sum_y = (float)(stack_point[stack_num - 1].y - stack_point[0].y) / (float)stack_num;

    if (stack_num < 4)
        return Point(-sum_x / 2, -sum_y / 2);
    else
        return Point(-sum_x, -sum_y);
}

bool tracking_dot::is_empty()
{
    if (tag == -1)
        return true;
    else
        return false;
}

void tracking_dot::goto_next_point()
{
    Point v = cal_v();
    p += v;
}

Point tracking_dot::predict_next_point()
{
    Point v = cal_v();
    return p + v;
}

int tracking_dot::which_thing_is_my_thing(vector<thing_info> current_thing, int distance_limit, int score_limit)
{
    vector<float> dis_list;
    vector<int> index_list;
    float score = -1;

    for (int i = 0; i < current_thing.size(); i++)
        dis_list.push_back(cal_distance(this, current_thing[i])); //current thing의 위치랑 똑같이 저장 됨

    index_list = get_least_dis_index_list(dis_list, distance_limit);
    sort(index_list.begin(), index_list.end());

    if (index_list.size() == 1)
        return index_list[0];
    else if (index_list.size() > 1)
    {
        for (int i = 0; i < index_list.size(); i++)
        {
            cout << "align_Images start" << endl;
            //cout << current_thing[index_list[i]].im.cols << " " << input.im.cols << endl;
            score = align_Images(current_thing[index_list[i]].im, im);

            if (score < score_limit && (score >= 0))
                return index_list[i];
        }
        return index_list[0];
    }
    else
        return -1;
}

int tracking_dot::update_dot(vector<thing_info> current_thing)
{
    float score_limit = 5;
    float distance_limit = 100;

    int flag = which_thing_is_my_thing(current_thing, distance_limit, score_limit);
    //일치하는 물체의 current_thing 인덱스를 반환

    if (flag != -1 && current_thing[flag].hit != 1) //flag 가 -1이 아닌경우는 which_thing_is_my_thing가 잘못되어서 나왔던것!!
    {
        cout << "im_copy start" << endl;
        for (int i = 0; i < current_thing.size(); i++)
        {
            cout << current_thing.size() << " "
                 << current_thing[i].name << " "
                 << current_thing[i].im.cols << " "
                 << current_thing[i].im.rows << " "
                 << flag << " "
                 << endl;
        }
        im = current_thing[flag].im;
        cout << "im_copy end" << endl;
        bbox = current_thing[flag].bbox;
        p = cal_center_point(current_thing[flag].bbox);
        name = current_thing[flag].name;
        is_missed = false;
        miss_stack = 0;
        velocity = cal_v();
    }
    else
    {
        p = predict_next_point();
        is_missed = true;
        miss_stack++;
    }
    put_point_to_stack(p);

    return flag;
}