#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

class ColorDetect
{
public:
	ColorDetect();
	bool detect(Mat frame, Rect& box);
private:
	int iLowH;
	int iHighH;

	int iLowS; 
	int iHighS;

	int iLowV;
	int iHighV;
	vector<Point> findMax(vector<vector<Point>> contours);
};

ColorDetect::ColorDetect()
{
	iLowH = 0;
	iHighH = 33;

	iLowS = 90; 
	iHighS = 255;

	iLowV = 90;
	iHighV = 255;

	namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"
	//Create trackbars in "Control" window
	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);
}

bool ColorDetect::detect(Mat frame, Rect& box)
{
	Mat imgHSV, imgThreshold;
	vector<Mat> channels(3); 
	cvtColor(frame, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

	//因为我们读取的是彩色图，直方图均衡化需要在HSV空间做
	split(imgHSV, channels);
	equalizeHist(channels[2],channels[2]);
	merge(channels,imgHSV);

	inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThreshold); //Threshold the image

	//开操作 (去除一些噪点)
	Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
	morphologyEx(imgThreshold, imgThreshold, MORPH_OPEN, element);

	//闭操作 (连接一些连通域)
	morphologyEx(imgThreshold, imgThreshold, MORPH_CLOSE, element);

	//imshow("Thresholded Image", imgThreshold); //show the thresholded image
	

	//寻找最外层轮廓  
	vector<vector<Point>> contours;  
	vector<Vec4i> hierarchy;  
	findContours(imgThreshold, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point());

	if(contours.size() == 0)
		return false;

	vector<Point> maxContour;
	maxContour = findMax(contours);

	box = boundingRect(maxContour);
	return true;
}

vector<Point> ColorDetect::findMax(vector<vector<Point>> contours)
{
	int index = 0;
	double maxArea = contourArea(contours[0]);
	for(size_t i = 1; i < contours.size(); i++)
	{
		double area = contourArea(contours[i]);
		if(area > maxArea)
		{
			index = i;
			maxArea = area;
		}
	}
	return contours[index];
}