#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

void detectAndDraw(Mat& img, CascadeClassifier& cascade, double scale, bool tryflip);

int main(int argc, const char** argv)
{
    VideoCapture capture;
    Mat frame;
    CascadeClassifier cascade;
    double scale = 1.1;
    bool tryflip = false;
    TickMeter t;

    string cascadeName = "cascade.xml";
    string inputName = "new.avi";

    if( !cascade.load( cascadeName ) )
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        return -1;
    }
    if(!capture.open( inputName ))
    {
        cout << "ERROR: Could not read " << inputName << endl;
        return -1;
    }
        
    if( capture.isOpened() )
    {
        cout << "Video capturing has been started ..." << endl;
        while(true)
        {
            capture >> frame;
            if(frame.empty())
                break;
            Mat frame1 = frame.clone();
            t.start();
            detectAndDraw(frame1, cascade, scale, tryflip);
            t.stop();
            char c = (char) waitKey(10);
            if(c == 27)
                break;
        }
    }

    double average_fps = t.getCounter() / t.getTimeSec();
    cout << "Average fps: " << average_fps << endl;
    return 0;
}

void detectAndDraw(Mat& img, CascadeClassifier& cascade, double scale, bool tryflip)
{
    vector<Rect> boards, boards2;
    Mat gray, smallImg;

    
    cvtColor(img, gray, COLOR_BGR2GRAY);
    double fx = 1 / scale;
    resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR);
    equalizeHist(smallImg, smallImg);

    cascade.detectMultiScale(smallImg, boards, 
                            1.1, 2, 0|CASCADE_SCALE_IMAGE, 
                            Size(30, 30), Size(80, 80));
    if(tryflip)
    {
        flip(smallImg, smallImg, 1);
        cascade.detectMultiScale(smallImg, boards2, 
                                1.1, 2, 0|CASCADE_SCALE_IMAGE, 
                                Size(50, 50), Size(100, 100));
        for(vector<Rect>::const_iterator r = boards2.begin(); r != boards2.end(); r++)
        {
            boards.push_back(Rect(smallImg.cols - r->x - r->width, 
                                    r->y, r->width, r->height));
        }
    }
    for(size_t i = 0; i < boards.size(); i++)
    {
        Rect r = boards[i];
        rectangle(img, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)),
                cvPoint(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                Scalar(255,128,0), 3, 8, 0);
    }
    imshow("result", img);
}
