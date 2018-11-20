#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

#include "utils.h"

using namespace std;

int main(int argc, char **argv)
{
    // List of tracker types in OpenCV 3.2
    // NOTE : GOTURN implementation is buggy and does not work.
    string trackerTypes[7] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN", "CSRT"};
    // vector <string> trackerTypes(types, std::end(types));

    // Create a tracker
    string trackerType = trackerTypes[3];

    cv::Ptr<cv::Tracker> tracker;

    if (trackerType == "BOOSTING")
        tracker = cv::TrackerBoosting::create();
    if (trackerType == "MIL")
        tracker = cv::TrackerMIL::create();
    if (trackerType == "KCF")
        tracker = cv::TrackerKCF::create();
    if (trackerType == "TLD")
        tracker = cv::TrackerTLD::create();
    if (trackerType == "MEDIANFLOW")
        tracker = cv::TrackerMedianFlow::create();
    if (trackerType == "GOTURN")
        tracker = cv::TrackerGOTURN::create();
    if (trackerType == "CSRT")
        tracker = cv::TrackerCSRT::create();

    // Create a detector
    string cfgDir = "cfg/yolov3-head.cfg";
    string weightDir = "cfg/yolov3-head.weights";
    Detector detector(cfgDir, weightDir);

    // Create a video stream
    cv::VideoCapture capture( 0 );  
    if(!capture.isOpened())
    {
        cout << "Could not read camera" << endl;
        return 1;
        
    }

    cv::TickMeter meter;
    cv::Mat frame, frameShow;
    // 0：No object.
    // 1：Object detected and tracking.
    int status = 0;     
    cv::Rect2d location;


    while(capture.read(frame))
    {
        meter.start();
        if(frame.empty())
        {
            cout << "No captured frame" << endl;
            break;
        }
            frameShow = frame.clone();


        double timer = (double)cv::getTickCount();
        if(status == 0)
        {
            vector<bbox_t> results = detector.detect(frame, 0.15);
            vector<cv::Rect2d> persons = person_filter(results);
            if(persons.size() > 0)
            {
                cout << "[debug] " << meter.getCounter() << ": " << "persons: " << persons.size() << endl;
                for(size_t i = 0; i < persons.size(); i++)
                    rectangle( frameShow, persons[i], cv::Scalar(20, 20, 20), 1, 8, 0);
                location = max_filter(persons);

                tracker->init(frame, location);
                status = 1;
                cout << "[debug] " << meter.getCounter() << ": " << "开始跟踪" << endl;
            }
        }
        else if(status == 1)
        {
            bool ok = tracker->update(frame, location);
            if( ok == false )
            {
                cout << "---------------------------------------Track failed" << endl;
                // Tracking failure detected. YOLO detecion
                vector<bbox_t> results = detector.detect(frame, 0.15);
                vector<cv::Rect2d> persons = person_filter(results);
                if(persons.size() > 0)
                {
                    // Detection success, reinit tracker.
                    for(size_t i = 0; i < persons.size(); i++)
                        rectangle( frameShow, persons[i], cv::Scalar(20, 20, 20), 1, 8, 0);
                    location = max_filter(persons);
                    tracker->init(frame, location);
                }
                else
                {
                    // Detection failed.
                    status = 0;
                }
            }
        }

        if(status == 1)
        {
            // Tracking or detection successed.
            cout << "[loc: " << location.x << " " << location.y  << " " << location.width << " " << location.height << "]\n";
            rectangle(frameShow, location, cv::Scalar(255, 0, 0), 2, 1);
        }
        else
        {
            // Tracking or detection failed.
            putText(frameShow, "Tracking failure detected", cv::Point(100,80), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0,0,255),2);
        }

        // Show real-time fps.
        float fps = cv::getTickFrequency() / ((double)cv::getTickCount() - timer);
        putText(frameShow, "FPS : " + SSTR(int(fps)), cv::Point(100,50), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(50,170,50), 2);
        imshow("Detection-Tracking", frameShow);

        int k = cv::waitKey(1);
        if(k == 27)
        {
            break;
        }

        meter.stop();
    }
    
    // Print average fps.
    double fps = double(meter.getCounter()) / meter.getTimeSec();
    cout << "[Average fps] " << fps << endl;

    return 0;
}