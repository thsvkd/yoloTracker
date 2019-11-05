#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    int on = atoi(argv[1]);

    string thsvkd_on[4];
    string base("sshpass -p thsxogud1 ssh -o StrictHostKeyChecking=no thsvkd");
    string ip("114.70.22.21 -p400");
    string program(" ./yoloTracker/video_streaming/videoserver 500");
    string off("pkill -x videoserver");

    for (int i = 0; i < 4; i++)
    {
        if (i == 0)
            thsvkd_on[0] += base + "@" + ip + to_string(i) + program + to_string(i);
        else
            thsvkd_on[0] += base + to_string(i) + "@" + ip + to_string(i) + program + to_string(i);

        if (on == 1)
            system((thsvkd_on[i] + "\n").c_str());
        else
            system((off + "\n").c_str());
    }

    return 0;
}
