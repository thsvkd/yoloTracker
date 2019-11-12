#include "tracker.h"

using namespace std;
using namespace cv;

extern int width, height;

void quadrant::push_thing(int pos, int tag)
{
    this->pos.push_back(pos);
    this->tag.push_back(tag);
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

    pos.erase(pos.begin() + poss);
    tag.erase(tag.begin() + poss);

    return result;
}
