#include "tracker.h"

using namespace std;
using namespace cv;

extern int width, height;
extern quadrant Quadrant;
extern vector<thing_color> tag_color;

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
    for (int i = stack_point.size() - 1; i > 0; i--)
        stack_point[i] = stack_point[i - 1];

    stack_point[0] = pp;
}

Point tracking_dot::cal_v()
{
    float sum_x = 0, sum_y = 0;
    int stack_n = 0;

    for (int i = 0; i < stack_point.size() - 1; i++)
        if (stack_point[i].x != -1)
            stack_n++;

    sum_x = (float)(stack_point[stack_n - 1].x - stack_point[0].x) / (float)stack_n;
    sum_y = (float)(stack_point[stack_n - 1].y - stack_point[0].y) / (float)stack_n;

    if (stack_n < 4)
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
        dis_list.push_back(cal_distance(p, current_thing[i])); //current thing의 위치랑 똑같이 저장 됨

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

//this func update tracking_dot's state
int tracking_dot::update_dot(Mat frame, vector<thing_info> current_thing)
{
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
    else //miss
    {
        p = predict_next_point();
        bbox.x = (float)p.x / (float)width - bbox.w / 2;
        bbox.y = (float)p.y / (float)height - bbox.h / 2;
        is_missed = true;
        miss_stack++;
    }
    put_point_to_stack(p);

    return flag;
}

bool tracking_dot::mosse_update(Mat frame)
{
    Rect2d tmp = box_to_Rect2d(bbox);
    is_mosse_updated = tracker->update(frame, tmp);
    bbox = Rect2d_to_box(tmp);

    return is_mosse_updated;
}

bool tracking_dot::if_tracker_get_out_screen()
{
    Point p_tmp = p;
    int left;
    int right;
    int top;
    int bot;

    if (position == 1)
    {
        p_tmp.x -= width / 2;
        left = 20;
        right = 0;
        top = 0;
        bot = 20;

        if (p_tmp.x < left && velocity.x < 0)
        {
            Quadrant.push_thing(3, tag, box_color);
            return true;
        }
        else if (p_tmp.x > width / 2 - right && velocity.x > 0)
        {
            return true;
        }
        else if (p_tmp.y < top)
        {
            return true;
        }
        else if (p_tmp.y > height / 2 - bot && velocity.y > 0)
        {
            Quadrant.push_thing(3, tag, box_color);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (position == 2)
    {
        left = 0;
        right = 20;
        top = 0;
        bot = 20;

        if (p_tmp.x < left)
        {
            return true;
        }
        else if (p_tmp.x > width / 2 - right && velocity.x > 0)
        {
            Quadrant.push_thing(4, tag, box_color);
            return true;
        }
        else if (p_tmp.y < top)
        {
            return true;
        }
        else if (p_tmp.y > height / 2 - bot && velocity.y > 0)
        {
            Quadrant.push_thing(4, tag, box_color);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (position == 3)
    {
        p_tmp.y -= height / 2;
        left = 20;
        right = 20;
        top = 0;
        bot = 20;

        if (p_tmp.x < left && velocity.x < 0)
        {
            Quadrant.push_thing(4, tag, box_color); ///여기부분 고쳐라!
            return true;
        }
        else if (p_tmp.x > width / 2 - right && velocity.x > 0)
        {
            Quadrant.push_thing(1, tag, box_color);
            return true;
        }
        else if (p_tmp.y < top)
        {
            return true;
        }
        else if (p_tmp.y > height / 2 - bot && velocity.y > 0)
        {
            Quadrant.push_thing(1, tag, box_color);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (position == 4)
    {
        p_tmp.x -= width / 2;
        p_tmp.y -= height / 2;
        left = 20;
        right = 60;
        top = 0;
        bot = 50;

        if (p_tmp.x < left && velocity.x < 0)
        {
            Quadrant.push_thing(2, tag, box_color);
            return true;
        }
        else if (p_tmp.x > width / 2 - right && velocity.x > 0)
        {
            Quadrant.push_thing(3, tag, box_color);
            return true;
        }
        else if (p_tmp.y < top)
        {
            return true;
        }
        else if (p_tmp.y > height / 2 - bot && velocity.y > 0)
        {
            Quadrant.push_thing(3, tag, box_color);
            return true;
        }
        else
        {
            return false;
        }
    }
}

int tracking_dot::what_quadrant_am_i()
{
    if (p.x < width / 2)
    {
        if (p.y < height / 2)
            position = 2;
        else
            position = 3;
    }
    else
    {
        if (p.y < height / 2)
            position = 1;
        else
            position = 4;
    }

    return position;
}