#include "staple_tracker.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <numeric>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "predict_car.h"

void limitRect(cv::Rect &location, cv::Size sz)
{
    cv::Rect window(cv::Point(0, 0), sz);
    location = location & window;
}

void find_circle(Mat &img, vector< vector<Point> > &contours)
{
    Mat img_gray;
    cvtColor(img, img_gray, CV_BGR2GRAY);
    Mat img_thresh;
    threshold(img_gray, img_thresh, 120, 255, CV_THRESH_BINARY);
    imshow("Threshold", img_thresh);
    findContours(img_thresh, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
    cout << "[debug] num: " << contours.size() << endl;
    double maxarea = img.rows * img.cols * 0.9;
    double minarea = img.rows * img.cols * 0.15;
    cout << "[debug] maxarea: " << maxarea << "  minarea: " << minarea << endl;
    for(int i = 0; i < contours.size(); i++)
    {
        double area = fabs(contourArea(contours[i]));
        cout << "[debug] " << i << ": " << area << endl;
        if(area < minarea || area > maxarea)
        {
            contours.erase(contours.begin() + i);
            i--;
        }
    }
    drawContours(img, contours, -1, Scalar(0, 200, 200), 3);
}

int main(int argc, char * argv[])
{
    //VideoWriter output_dst("detectracker.avi", CV_FOURCC('M', 'J', 'P', 'G'), 23, Size(640, 480), 1);

    CarPredict net;
    STAPLE_TRACKER staple;
    cv::VideoCapture capture;
    capture.open("blue.avi");
    if(!capture.isOpened())
    {
        std::cout << "fail to open" << std::endl;
        exit(0);
    }
    cv::String cascade_name = "cascade.xml";;
    cv::CascadeClassifier detector;
    if( !detector.load( cascade_name ) )
    { 
        printf("--(!)Error loading face cascade\n"); 
        return -1; 
    };


    int64 tic, toc;
    double time = 0;
    bool show_visualization = true;
    int status = 0;     //0：没有目标，1：找到目标进行跟踪
    cv::Mat frame;
    int frame_num = 0;
    Rect location;
    std::vector<cv::Rect_<float>> result_rects;
    while ( capture.read(frame) )
    {
        if( frame.empty() )
        {
            printf(" --(!) No captured frame -- Break!");
            break;
        }
        frame_num++;
        tic = cv::getTickCount();
        if(status == 0)
        {
            //cout << "[debug] " << frame_num << ":" << " 没有目标" << endl;
            std::vector<Rect> boards;
            std::vector<Rect> true_boards;
            cv::Mat frame_gray;
            cv::cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
            detector.detectMultiScale( frame_gray, boards, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(60, 60) ,Size(100, 100));
            if(boards.size() > 0)
            {
                cout << "[debug] " << frame_num << ":" << " cascade找到" << boards.size() << "个目标" << endl;
                for(int i = 0; i < boards.size(); i++)
                {
                    Rect roi = Rect(Point(boards[i].x, boards[i].y), Size(boards[i].width, boards[i].height));
                    Mat board = frame(roi);
                    net.predict(board);
                    if(net.getId() == 0)
                    {
                        // rectangle( frame, cvPoint(cvRound(boards[i].x), cvRound(boards[i].y)),
                        //         cvPoint(cvRound((boards[i].x + boards[i].width-1)), cvRound((boards[i].y + boards[i].height-1))),
                        //         Scalar( 20, 20, 255 ), 3, 8, 0);
                        true_boards.push_back(boards[i]);
                    }
                }
            }
            if(true_boards.size() > 0)
            {
                cout << "[debug] " << frame_num << ":" << " 用cnn过滤后还有" << true_boards.size() << "个目标" << endl;
                for(int i = 0; i < true_boards.size(); i++)
                {
                    rectangle( frame, cvPoint(cvRound(true_boards[i].x), cvRound(true_boards[i].y)),
                             cvPoint(cvRound((true_boards[i].x + true_boards[i].width-1)), cvRound((true_boards[i].y + true_boards[i].height-1))),
                             Scalar( 20, 20, 20 ), 3, 8, 0);
                }
                cv::imshow("detectracker", frame);
                waitKey(0);
                if(true_boards.size() == 1)
                    location = true_boards[0];
                else
                {
                    int max_area = true_boards[0].width * true_boards[0].height;
                    int max_index = 0;
                    for(int i = 1; i < true_boards.size(); i++)
                    {
                        int area = true_boards[i].width * true_boards[i].height;
                        if(area > max_index)
                        {
                            max_area = area;
                            max_index = i;
                        }
                    }
                    location = true_boards[max_index];
                }
                staple.tracker_staple_initialize(frame, location);
                staple.tracker_staple_train(frame, true);
                status = 1;
                cout << "[debug] " << frame_num << ":" << " 开始跟踪" << endl;
            }
        }
        else if(status == 1)
        {
            location = staple.tracker_staple_update(frame);
            limitRect(location, frame.size());
            if(location.empty())
            {
                status = 0;
                continue;
            }
            // if(frame_num % 3 == 0)
            // {
            //     Mat roi = frame(location);
            //     imwrite("../data/" + to_string(frame_num) + ".jpg", roi);
            // }
            
            staple.tracker_staple_train(frame, false);
            result_rects.push_back(location);
            if(frame_num % 10 == 0)
            {
                Mat board = frame(location);
                net.predict(board);
                if(net.getId() == 1)
                {
                    cout << "[debug] " << frame_num << ":" << " 丢失目标" << endl;
                    status = 0;
                }
            }
            vector< vector<Point> > contours;
            Mat roi = frame(location);
            find_circle(roi, contours);
        }
        toc = cv::getTickCount() - tic;
        time += toc;

        if (show_visualization) {
            cv::putText(frame, std::to_string(frame_num), cv::Point(20, 40), 6, 1,
                cv::Scalar(0, 255, 255), 2);
            if(status == 1)
                cv::rectangle(frame, location, cv::Scalar(0, 128, 255), 2);
            cv::imshow("detectracker", frame);
            //output_dst << frame;

            char key = cv::waitKey(10);
            if (key == 27 || key == 'q' || key == 'Q')
                break;
        }
    }
    
    time = time / double(cv::getTickFrequency());
    double fps = double(frame_num) / time;
    std::cout << "fps:" << fps << std::endl;
    cv::destroyAllWindows();

    return 0;
}

