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