#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

class Template{
    public:
        Template();
        void initTracking(Mat frame, Rect box, int scale = 5);
        Rect track(Mat frame);
        Rect getLocation();
    private:
        Mat model;
        Rect location;
        int scale;
};

Template::Template()
{
    this->scale = 5;
}

void Template::initTracking(Mat frame, Rect box, int scale)
{
    this->location = box;
    this->scale = scale;
    this->location &= Rect(0, 0, frame.cols, frame.rows);

    if(frame.empty())
    {
        cout << "ERROR: frame is empty." << endl;
        exit(0);
    }
    if(frame.channels() != 1)
    {
        cvtColor(frame, frame, CV_RGB2GRAY);
    }
    this->model = frame(this->location);
}

Rect Template::track(Mat frame)
{
    if(frame.empty())
    {
        cout << "ERROR: frame is empty." << endl;
        exit(0);
    }
    Mat gray;
    if(frame.channels() != 1)
    {
        cvtColor(frame, gray, CV_RGB2GRAY);
    }

    Rect searchWindow;
    searchWindow.width = this->location.width * scale;
    searchWindow.height = this->location.height * scale;
    searchWindow.x = this->location.x + this->location.width * 0.5 
                        - searchWindow.width * 0.5;
    searchWindow.y = this->location.y + this->location.height * 0.5
                        - searchWindow.height * 0.5;
    
    if(searchWindow.x < 0)
        searchWindow.x = 0;
    if(searchWindow.y < 0)
        searchWindow.y = 0;
    searchWindow &= Rect(0, 0, frame.cols, frame.rows);
    
    Mat similarity;
    matchTemplate(gray(searchWindow), this->model, similarity, CV_TM_CCOEFF_NORMED);
    double mag_r;
    Point point;
    minMaxLoc(similarity, 0, &mag_r, 0, &point);
    this->location.x = point.x + searchWindow.x;
    this->location.y = point.y + searchWindow.y;

    this->model = gray(location);
    return this->location;
}

Rect Template::getLocation()
{
    return this->location;
}