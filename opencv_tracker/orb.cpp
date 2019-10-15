#include "tracker.h"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

const int MAX_FEATURES = 500;
const float GOOD_MATCH_PERCENT = 0.15f;
const float GOOD_ANGLE_PERCENT = 0.9f;

// int main(int argc, char **argv)
// {
//     // Read reference image
//     string refFilename("7.png");
//     cout << "Reading reference image : " << refFilename << endl;
//     Mat imReference = imread(refFilename);

//     // Read image to be aligned
//     string imFilename("8.png");
//     cout << "Reading image to align : " << imFilename << endl;
//     Mat im = imread(imFilename);

//     // Registered image will be resotred in imReg.
//     // The estimated homography will be stored in h.
//     Mat imReg, h;

//     // Align images
//     cout << "Aligning images ..." << endl;
//     alignImages(im, imReference, imReg, h);

//     // Write aligned image to disk.
//     string outFilename("aligned.jpg");
//     cout << "Saving aligned image : " << outFilename << endl;
//     imwrite(outFilename, imReg);

//     // Print estimated homography
//     cout << "Estimated homography : \n"
//          << h << endl;
// }

float doublecalcAngleFromPoints(Point2f _ptFirstPos, Point2f _ptSecondPos)
{
    float fAngle;
    float fdX = _ptFirstPos.x - _ptSecondPos.x;
    float fdY = _ptFirstPos.y - _ptSecondPos.y;

    float dRad = atan2(fdY, fdX);
    fAngle = (dRad * 180) / 3.14159265;
    if (fAngle >= 90)
        fAngle -= 180;
    if (fAngle <= -90)
        fAngle += 180;
    return fAngle;
}

void alignImages(Mat &im1, Mat &im2, Mat &im1Reg, Mat &h)
{
    // Convert images to grayscale
    Mat im1Gray, im2Gray;
    cout << "cvtColor start" << endl;
    cvtColor(im1, im1Gray, CV_BGR2GRAY);
    cvtColor(im2, im2Gray, CV_BGR2GRAY);

    // Variables to store keypoints and descriptors
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;

    // Detect ORB features and compute descriptors.
    cout << "detectAndCompute start" << endl;
    Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);
    orb->detectAndCompute(im1Gray, Mat(), keypoints1, descriptors1);
    orb->detectAndCompute(im2Gray, Mat(), keypoints2, descriptors2);

    // Match features.
    std::vector<DMatch> matches;
    cout << "match start" << endl;
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
    matcher->match(descriptors1, descriptors2, matches, Mat());

    // Sort matches by score
    std::sort(matches.begin(), matches.end());

    // Remove not so good matches
    const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
    matches.erase(matches.begin() + numGoodMatches, matches.end());

    float same = 0;
    float avg = 0;

    // Draw top matches
    Mat imMatches;
    cout << "drawMatches start" << endl;
    drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);
    imwrite("matches.jpg", imMatches);

    // Extract location of good matches
    std::vector<Point2f> points1, points2;

    for (size_t i = 0; i < matches.size(); i++)
    {
        points1.push_back(keypoints1[matches[i].queryIdx].pt);
        points2.push_back(keypoints2[matches[i].trainIdx].pt);
        points2[i].x += im1.cols;
    }

    vector<float> angle_f;

    for (int i = 0; i < points1.size(); i++)
        angle_f.push_back(abs(doublecalcAngleFromPoints(points1[i], points2[i])));

    const int numGoodAngleMatches = angle_f.size() * GOOD_ANGLE_PERCENT;
    std::sort(angle_f.begin(), angle_f.end());
    angle_f.erase(angle_f.begin() + numGoodAngleMatches, angle_f.end());

    for (int i = 0; i < angle_f.size(); i++)
        avg += angle_f[i];
    avg /= angle_f.size();
    for (int i = 0; i < angle_f.size(); i++)
        same += pow(angle_f[i] - avg, 2);
    same /= angle_f.size();

    cout << sqrt(same) << std::endl;

    // Find homography
    h = findHomography(points1, points2, RANSAC);

    // Use homography to warp image
    warpPerspective(im1, im1Reg, h, im2.size());
}

float align_Images(Mat &im1, Mat &im2)
{
    // Convert images to grayscale
    Mat im1Gray, im2Gray;
    cout << "cvtColor start" << endl;
    cout << im1.cols << " " << im2.cols << endl;
    cvtColor(im1, im1Gray, CV_BGR2GRAY);
    cvtColor(im2, im2Gray, CV_BGR2GRAY);
    cout << im1Gray.cols << " " << im2Gray.cols << endl;

    // Variables to store keypoints and descriptors
    std::vector<KeyPoint> keypoints1, keypoints2;
    Mat descriptors1, descriptors2;

    // Detect ORB features and compute descriptors.
    Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);
    cout << "detectAndCompute start" << endl;
    orb->detectAndCompute(im1Gray, Mat(), keypoints1, descriptors1);
    orb->detectAndCompute(im2Gray, Mat(), keypoints2, descriptors2);

    // Match features.
    std::vector<DMatch> matches;
    cout << "match start" << endl;
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
    cout << descriptors1.cols << " " << descriptors2.cols << endl;
    matcher->match(descriptors1, descriptors2, matches, Mat());
    cout << "a start" << endl;

    // Sort matches by score
    std::sort(matches.begin(), matches.end());

    // Remove not so good matches

    const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
    matches.erase(matches.begin() + numGoodMatches, matches.end());

    float same = 0;
    float avg = 0;

    // Draw top matches
    Mat imMatches;
    cout << "drawMatches start" << endl;
    drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);
    imwrite("matches.jpg", imMatches);

    // Extract location of good matches
    std::vector<Point2f> points1, points2;

    for (size_t i = 0; i < matches.size(); i++)
    {
        points1.push_back(keypoints1[matches[i].queryIdx].pt);
        points2.push_back(keypoints2[matches[i].trainIdx].pt);
        points2[i].x += im1.cols;
    }

    for (int i = 0; i < points1.size(); i++)
        avg += doublecalcAngleFromPoints(points1[i], points2[i]);
    avg /= points1.size();
    for (int i = 0; i < points1.size(); i++)
        same += pow(doublecalcAngleFromPoints(points1[i], points2[i]) - avg, 2);
    same /= points1.size();

    return sqrt(same);
}
