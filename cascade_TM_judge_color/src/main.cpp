#include "template.h"
#include "serial.h"
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

void limitRect(cv::Rect &location, cv::Size sz)
{
    cv::Rect window(cv::Point(0, 0), sz);
    location = location & window;
}

bool get_color();	//true is red, false is blue
bool judge_color(Mat src);
vector<Rect> color_filter(Mat frame, vector<Rect> boards, bool color_flag);	//color filter

int main(int argc, char * argv[])
{
    //VideoWriter output_dst("newtracker.avi", CV_FOURCC('M', 'J', 'P', 'G'), 60, Size(640, 480), 1);
    bool color_flag = false;
    //color_flag = get_color();
    
    string dev = "/dev/ttyUSB0";
    Serial sel((char *)dev.data());
    sel.setPara(115200);

    Template tracker;
    cv::VideoCapture capture;
    capture.open(0);
    if(!capture.isOpened())
    {
        std::cout << "fail to open" << std::endl;
        exit(0);
    }
    cv::String cascade_name = "/home/ubuntu/Desktop/tracking/build/cascade.xml";;
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
	resize(frame, frame, Size(), 0.6, 0.8);
        frame_num++;
        tic = cv::getTickCount();
        if(status == 0)
        {
            //cout << "[debug] " << frame_num << ":" << " 没有目标" << endl;
            std::vector<Rect> boards;
            cv::Mat frame_gray;
            cv::cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
            detector.detectMultiScale( frame_gray, boards, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(10, 10) ,Size(130, 130));
	    boards = color_filter(frame, boards, color_flag);
            if(boards.size() > 0)
            {
                cout << "[debug] " << frame_num << ":" << " Detection find " << boards.size() << " objects" << endl;
                //for(int i = 0; i < boards.size(); i++)
                //{
                //    rectangle( frame, cvPoint(cvRound(boards[i].x), cvRound(boards[i].y)),
                //             cvPoint(cvRound((boards[i].x + boards[i].width-1)), cvRound((boards[i].y + boards[i].height-1))),
                //             Scalar( 20, 20, 20 ), 3, 8, 0);
                //}
                //cv::imshow("detectracker", frame);
                //waitKey(0);
                if(boards.size() == 1)
                    location = boards[0];
                else
                {
                    int max_area = boards[0].width * boards[0].height;
                    int max_index = 0;
                    for(int i = 1; i < boards.size(); i++)
                    {
                        int area = boards[i].width * boards[i].height;
                        if(area > max_index)
                        {
                            max_area = area;
                            max_index = i;
                        }
                    }
                    location = boards[max_index];
                }
                tracker.initTracking(frame, location);
                status = 1;
                cout << "[debug] " << frame_num << ":" << " Start tracking" << endl;
            }
        }
        else if(status == 1)
        {
            location = tracker.track(frame);
            limitRect(location, frame.size());
            if(location.area() == 0)
            {
                status = 0;
                continue;
            }
            // if(frame_num % 3 == 0)
            // {
            //     Mat roi = frame(location);
            //     imwrite("../data/" + to_string(frame_num) + ".jpg", roi);
            // }
            result_rects.push_back(location);
            if(frame_num % 20 == 0)
            {
                //在图片周围进行寻找
                int factor = 2;
                int newx = location.x + (1 - factor) * location.width / 2;
                int newy = location.y + (1 - factor) * location.height / 2;
                Rect loc = Rect(newx, newy, location.width * factor, location.height * factor);
                limitRect(loc, frame.size());
                Mat roi = frame(loc);
		cvtColor( roi, roi, COLOR_BGR2GRAY );
		//imshow("debug", roi);
                std::vector<Rect> boards;
                detector.detectMultiScale( roi, boards, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(), roi.size());
               
                if(boards.size() <= 0)
                {
                    status = 0;
		    cout << "[debug] " << frame_num << ": " << "Tracking loss objects" << endl;
                }
                else
                {
                    //location = boards[0];
                    location = Rect(boards[0].x + loc.x, boards[0].y + loc.y, boards[0].width, boards[0].height);
                    tracker.initTracking(frame,location);
                }
            }
        }
        toc = cv::getTickCount() - tic;
        time += toc;
	
	    if(status == 1)
	    {
cout << "nothing..." << endl;
		short int px = location.x + location.width / 2;
		short int py = location.y + location.height / 2;
		px -= frame.cols / 2;
		py -= frame.rows / 2;

		char buff[8];
		buff[0] = 0xff;
		buff[1] = 0xfe;
		buff[2] = 0x01;
		buff[3] = (px >> 8) & 0xff;
		buff[4] = px & 0xff;
		buff[5] = (py >> 8) & 0xff;
		buff[6] = py & 0xff;
		buff[7] = (buff[3] + buff[4] + buff[5] + buff[6]) % 0xff;
		sel.writeData(buff, 8);
		
		//cout << "[debug] " << px << ' ' << py << endl;
	    }
	    else
	    {
		char buff[8];
		buff[0] = 0xff;
		buff[1] = 0xfe;
		buff[2] = 0x00;
		buff[3] = 0x00;
		buff[4] = 0x00;
		buff[5] = 0x00;
		buff[6] = 0x00;
		buff[7] = 0x00;
		sel.writeData(buff, 8);
	    }

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

bool get_color()
{	
    string dev = "/dev/ttyUSB0";
    Serial sel((char *)dev.data());
    sel.setPara(115200);
    cout << "Waiting for color signal..." << endl;
    int len = 0;
    char buff[1];
    bool flag;
    while(true)
    {
	while(sel.readData(buff, 1) != 1);
	//cout << hex << (unsigned int) (unsigned char)buff[0] << ' ';
	if(buff[0] == 0xaa)
	{
	    //cout << "get oxaa" << endl;
	    while(sel.readData(buff, 1) != 1);
	    if(buff[0] == 0xaa)
	    {
		//cout << "get oxaa" << endl;
		while(sel.readData(buff, 1) != 1);
		if(buff[0] == 0x02)
		{
		    //cout << "get ox02" << endl;
		    while(sel.readData(buff, 1) != 1);
		    if(buff[0] == 0x06)
		    {
			//cout << "get oxa6" << endl;
			while(sel.readData(buff, 1) != 1);
			if(buff[0] == 0x00)
			{
			    while(sel.readData(buff, 1) != 1);
			    if(buff[0] == 0x01 )
			    {
				flag = true;	//red
				cout << "Get color: red" << endl;
				break;
			    }
			    else if(buff[0] == 0x00)
			    {
				flag = false;	//blue
				cout << "Get color: blue" << endl;
				break;
			    }
			}
		    }
		}
	    }
	}
    }
    return flag;
}

bool judge_color(Mat src) 
{
    int blue_count = 0;
    int red_count = 0;
    for(int i = 0; i < src.rows; i++)
    {
	for(int j = 0; j < src.cols; j++)
	{
	    if( src.at<cv::Vec3b>(i, j)[0] > 17 && src.at<cv::Vec3b>(i, j)[0] < 50 &&
		src.at<cv::Vec3b>(i, j)[1] > 15 && src.at<cv::Vec3b>(i, j)[1] < 56 &&
		src.at<cv::Vec3b>(i, j)[2] > 100 && src.at<cv::Vec3b>(i, j)[2] < 250 )
		red_count++;
	    else if( 
		src.at<cv::Vec3b>(i, j)[0] > 86 && src.at<cv::Vec3b>(i, j)[0] < 220 &&
		src.at<cv::Vec3b>(i, j)[1] > 31 && src.at<cv::Vec3b>(i, j)[1] < 88 &&
		src.at<cv::Vec3b>(i, j)[2] > 4 && src.at<cv::Vec3b>(i, j)[2] < 50 )
		blue_count++;
	}
    }
    cout << "[debug] " << "blue_count: " << blue_count << "\tred_count: " << red_count << endl;
    if(red_count > blue_count)
	return true;
    else
	return false;
}

vector<Rect> color_filter(Mat frame, vector<Rect> boards, bool color_flag)	//color filter
{
    vector<Rect> results;
    for(int i = 0; i < boards.size(); i++)
    {
	Mat roi = frame(boards[i]);
	//imshow("roi", roi);
	//waitKey(0);
	bool flag = judge_color(roi);
	if(flag == color_flag)
	    results.push_back(boards[i]);
    }
    //cout << results.size() << endl;
    return results;
}

