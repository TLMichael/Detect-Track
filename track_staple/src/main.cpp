#include "staple_tracker.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <numeric>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

bool gotBB = false;
Rect box;
bool drawingBox = true;

void onMouse(int event, int x, int y, int flags, void *param)
{
    switch(event)
    {
        case CV_EVENT_MOUSEMOVE:
            if (drawingBox)
            {
                box.width = x - box.x;
                box.height = y - box.y;
            }
            break;
        case CV_EVENT_LBUTTONDOWN:
            drawingBox = true;
            box = Rect(x, y, 0, 0);
            break;
        case CV_EVENT_LBUTTONUP:
            drawingBox = false;
            if(box.width < 0)
            {
                box.x +=box.width;
                box.width *= -1;
            }
            if(box.height < 0)
            {
                box.y += box.height;
                box.height *= -1;
            }
            gotBB = true;
            break;
    }
}

int main(int argc, char * argv[])
{
    cv::VideoCapture capture;
    capture.open( 0 );
    if(!capture.isOpened())
    {
        std::cout << "fail to open" << std::endl;
        exit(0);
    }
    //int frame_number = capture.get(CV_CAP_PROP_FRAME_COUNT);
    
    cvNamedWindow("Tracker", CV_WINDOW_AUTOSIZE);
    cvSetMouseCallback("Tracker", onMouse, NULL);

    Mat image;
    capture >> image;
    while (!gotBB)
    {
        capture >> image;

        imshow("Tracker", image);
        if (cvWaitKey(10) == 'q')
            return 1;
    }
    cvSetMouseCallback("Tracker", NULL, NULL);


    STAPLE_TRACKER staple;

    cv::Rect_<float> location;
    location = box;
    staple.tracker_staple_initialize(image, location);
    staple.tracker_staple_train(image, true);

    std::vector<cv::Rect_<float>> result_rects;
    int64 tic, toc;
    double time = 0;
    bool show_visualization = true;

    int frame = 0;
    while(true)
    {
        if(!capture.read(image))
            break;

        tic = cv::getTickCount();
        location = staple.tracker_staple_update(image);
        staple.tracker_staple_train(image, false);
        toc = cv::getTickCount() - tic;
        time += toc;
        result_rects.push_back(location);

        if (show_visualization) {
            cv::putText(image, std::to_string(frame++ + 1), cv::Point(20, 40), 6, 1,
                cv::Scalar(0, 255, 255), 2);
            cv::rectangle(image, location, cv::Scalar(0, 128, 255), 2);
            cv::imshow("STAPLE", image);

            char key = cv::waitKey(10);
            if (key == 27 || key == 'q' || key == 'Q')
                break;
        }
    }

    time = time / double(cv::getTickFrequency());
    double fps = double(result_rects.size()) / time;
    std::cout << "fps:" << fps << std::endl;
    cv::destroyAllWindows();

    return 0;
}

