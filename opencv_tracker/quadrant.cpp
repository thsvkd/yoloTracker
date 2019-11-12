#include "tracker.h"

using namespace std;
using namespace cv;

extern int width, height;

void quadrant::push_thing(int pos, int tag, Scalar box_color)
{
    this->pos.push_back(pos);
    this->tag.push_back(tag);
    this->thing_color.push_back(box_color);
}

vector<int> quadrant::pop_thing()
{
    vector<int> result;

    result.push_back(tag[tag.size() - 1]);
    result.push_back(tag[tag.size() - 1]);

    tag.pop_back();
    pos.pop_back();

    return result;
}

vector<int> quadrant::still_thing(int poss)
{
    vector<int> result;

    result.push_back(pos[poss]);
    result.push_back(tag[poss]);
    result.push_back(thing_color[poss].val[0]);
    result.push_back(thing_color[poss].val[1]);
    result.push_back(thing_color[poss].val[2]);

    pos.erase(pos.begin() + poss);
    tag.erase(tag.begin() + poss);
    thing_color.erase(thing_color.begin() + poss);

    return result;
}
