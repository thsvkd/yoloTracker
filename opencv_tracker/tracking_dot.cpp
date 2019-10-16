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
    for (int i = 0; i < stack_num - 1; i++)
    {
        stack_point[i + 1] = stack_point[i];
    }
    stack_point[0] = pp;
}

Point tracking_dot::cal_v()
{
    int sum_num = 0;
    float sum_x = 0, sum_y = 0;

    for (int i = 0; i < stack_num; i++)
    {
        if (stack_point[i].x != -1)
        {
            sum_x += stack_point[i].x;
            sum_y += stack_point[i].y;
            sum_num++;
        }
    }

    sum_x = (float)sum_x / (float)sum_num;
    sum_y = (float)sum_y / (float)sum_num;

    if (sum_num < 4)
        return Point(sum_x / 2, sum_y / 2);
    else
        return Point(sum_x, sum_y);
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

float tracking_dot::cal_distance_score(vector<thing_info> current_thing, int distance_limit)
{
    vector<float> dis_list;
    vector<int> index_list;
    float score = -1;

    for (int i = 0; i < current_thing.size(); i++)
        dis_list.push_back(cal_distance(this, current_thing[i]));

    index_list = get_least_dis_index_list(dis_list, distance_limit);
    sort(index_list.begin(), index_list.end());

    if (index_list.size() == 1)
        return index_list[0];
    else if (index_list.size() > 1)
    {
        for (int i = 0; i < index_list.size(); i++)
        {
            cout << "align_Images start" << endl;
            cout << trackers_dot[index_list[i]].im.cols << " " << input.im.cols << endl;
            score = align_Images(trackers_dot[index_list[i]].im, input.im);

            if (score < score_limit && (score >= 0))
                return index_list[i];
        }
    }
}

float tracking_dot::cal_matching_score(vector<thing_info> current_thing, int score_limit)
{
}

int tracking_dot::get_dot_thing(vector<thing_info> current_thing)
{
    float score_limit = 2.5;
    float distance_limit = 50;

    if (cal_distance_score(current_thing, distance_limit) < 50)
    {
    }
    else
    {
        if (cal_matching_score(current_thing, score_limit) < score_limit)
        {
        }
        else
        {
        }
    }
}