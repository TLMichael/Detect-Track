#include <iostream>
#include "utils.h"

using namespace std;
using namespace cv;

bool show_visualization = true;     //控制imshow的显示功能

int main()
{
    string cfgDir = "cfg/yolov3-head.cfg";
    string weightDir = "cfg/yolov3-head.weights";
    Detector detector(cfgDir, weightDir);
    Template tracker;

    VideoCapture capture;
    // capture.open( "http://192.168.0.6:8080/video" );
    capture.open( 0 );
    if(!capture.isOpened())
    {
        cout << " --(!) Fail to open camera -- Exit!" << endl;
        exit(0);
    }


    TickMeter meter;
    Mat frame;
    Mat frameShow;

    int status = 0;     // 0：没有目标，1：找到目标进行跟踪
    Rect location;

    while( capture.read(frame) )
    {
        if(frame.empty())
        {
            cout << " --(!) No captured frame -- Break!" << endl;
            break;
        }
        // resize(frame, frame, Size(640, 480));
        cv::putText(frame, GetCurrentTime2(), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
        frameShow = frame.clone();

        meter.start();
        if(status == 0)
        {
            vector<bbox_t> results = detector.detect(frame, 0.15);
            vector<Rect> persons = person_filter(results);
            if(persons.size() > 0)
            {
                cout << "[debug] " << meter.getCounter() << ": " << "persons: " << persons.size() << endl;
                for(size_t i = 0; i < persons.size(); i++)
                    rectangle( frameShow, persons[i], Scalar(20, 20, 20), 1, 8, 0);
                location = max_filter(persons);
                rectangle(frameShow, location, Scalar(0, 128, 255), 2);

                tracker.initTracking(frame, location);
                status = 1;
                cout << "[debug] " << meter.getCounter() << ": " << "开始跟踪" << endl;
            }
        }
        else if(status == 1)
        {
            if( meter.getCounter() % 3 != 0 )
            {
                location = tracker.track(frame);
                limitRect(location, frame.size());
                if(location.area() == 0)
                    status = 0;
            }
            else
            {
                vector<bbox_t> results = detector.detect(frame, 0.15);
                vector<Rect> persons = person_filter(results);
                if(persons.size() == 0)
                    status = 0;
                else
                {
                    location = max_filter(persons);
                    tracker.initTracking(frame, location);
                }
            }
            if(status == 1)
            {
                cout << "[loc: " << location.x << " " << location.y  << " " << location.width << " " << location.height << "]\n";
                rectangle(frameShow, location, Scalar(0, 128, 255), 2);
            }
        }
        
        if(show_visualization)
        {
            imshow("YinFeiLin", frameShow);
            char key = waitKey(1);
            if (key == 27 || key == 'q' || key == 'Q')
                break;
        }

        meter.stop();
    }

    double fps = double(meter.getCounter()) / meter.getTimeSec();
    cout << "[YinFeiLin] " << "fps: " << fps << endl;

    return 0;
}