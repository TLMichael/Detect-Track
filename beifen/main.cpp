#include "template.h"

#include <windows.h>
#include "serial.h"
#include <string.h>
#include <iostream>
#include <time.h>
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


// 采用多项式拟合，输出距离单位为米
// x的范围在30~130个像素之间
// d的范围在50~600厘米之间
short int pixel2distance(int x)
{
	x = 2 * x;
	float w0, w1, w2, w3;
	w0 = 7.8469;
	w1 = -0.1784;
	w2 = 0.0017;
	w3 = -5.4871 / 1000000;
	short int d = (short int)(100 * (w0 + w1 * x + w2 * x*x + w3 * x*x*x));
	return d;
}

//生成随机字符串
char *rand_str(char *str, const int len)
{
	srand(time(NULL));
	int i;
	for(i = 0; i < len - 5; i++)
	{
		switch((rand()%3))
		{
		case 1:
			str[i]='A'+rand()%26;
			break;
        case 2:
			str[i]='a'+rand()%26;
			break;
		default:
			str[i]='0'+rand()%10;
			break;
		}
    }
    str[len - 5] = '.';
	str[len - 4] = 'a';
	str[len - 3] = 'v';
	str[len - 2] = 'i';
	str[len - 1] = '\0';
    return str;
}


int main(int argc, char * argv[])
{
	//CSerial serial;
	//serial.OpenSerialPort(_T("COM6"),115200,8,1);  //打开串口后，自动接收数据
	char filename[10];	//用于保存视频文件
	rand_str(filename, 10);
    VideoWriter output_dst(filename, CV_FOURCC('M', 'J', 'P', 'G'), 15, Size(320, 240), 1);
    
	Template tracker;
    
	cv::VideoCapture capture;
    capture.open( 0 );
    if(!capture.isOpened())
    {
        std::cout << "fail to open" << std::endl;
        exit(0);
    }
    cv::String cascade_name = "haar-frontface.xml";;
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
    //std::vector<cv::Rect_<float>> result_rects;
    while ( capture.read(frame) )
    {
        if( frame.empty() )
        {
            printf(" --(!) No captured frame -- Break!");
            break;
        }
		resize(frame, frame, Size(), 0.5, 0.5);
		//cout << "[debug] " << frame.cols << ' ' << frame.rows << endl;
        frame_num++;
        tic = cv::getTickCount();
        if(status == 0)
        {
            //cout << "[debug] " << frame_num << ":" << " 没有目标" << endl;
            std::vector<Rect> boards;
            cv::Mat frame_gray;
            cv::cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
            detector.detectMultiScale( frame_gray, boards, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(20, 20) ,Size(80, 80));
            if(boards.size() > 0)
            {
                cout << "[debug] " << frame_num << ":" << " cascade找到" << boards.size() << "个目标" << endl;
                for(size_t i = 0; i < boards.size(); i++)
                {
                    rectangle( frame, cvPoint(cvRound(boards[i].x), cvRound(boards[i].y)),
                             cvPoint(cvRound((boards[i].x + boards[i].width-1)), cvRound((boards[i].y + boards[i].height-1))),
                             Scalar( 20, 20, 20 ), 3, 8, 0);
                }
                //cv::imshow("detectracker", frame);
                //waitKey(0);
                if(boards.size() == 1)
                    location = boards[0];
                else
                {
                    int max_area = boards[0].width * boards[0].height;
                    int max_index = 0;
                    for(size_t i = 1; i < boards.size(); i++)
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
                cout << "[debug] " << frame_num << ":" << " 开始跟踪" << endl;
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
            //result_rects.push_back(location);
            if(frame_num % 10 == 0)
            {
				//Rect loc = location
				int factor = 2;
				int newx = location.x + (1 - factor) * location.width / 2;
				int newy = location.y + (1 - factor) * location.height / 2;
				Rect loc = Rect(newx, newy, location.width * factor, location.height * factor);
				cv::rectangle(frame, loc, cv::Scalar(255, 128, 255), 2);
				limitRect(loc, frame.size());
                Mat roi = frame(loc);
                std::vector<Rect> boards;
                detector.detectMultiScale( roi, boards, 1.1, 2, 
                            0|CASCADE_SCALE_IMAGE, Size(location.width / 2, location.width / 2), roi.size());
                cout << "[debug] boards: " << boards.size() << endl;
                if(boards.size() <= 0)
                {
                    status = 0;
                }
				else
				{
					//location = boards[0];
					location = Rect(boards[0].x + loc.x, boards[0].y + loc.y, boards[0].width, boards[0].height);
					tracker.initTracking(frame, location);
				}
            }
        }
        toc = cv::getTickCount() - tic;
        time += toc;

		if(status == 1)
		{
			short int px = location.x + location.width / 2;
			short int py = location.y + location.height / 2;
			px -= frame.cols / 2;
			py -= frame.rows / 2;
			short int distance = pixel2distance(location.height);
			char buff[9];
			buff[0] = 0xFF;
			buff[1] = 0xFE;
			buff[2] = 0x01;
			buff[3] = (px >> 8) & 0xff;
			buff[4] = px & 0xff;
			buff[5] = (py >> 8) & 0xff;
			buff[6] = py & 0xff;
			buff[7] = (distance >> 8) & 0xff;
			buff[8] = distance & 0xff;
			buff[9] = (buff[3] + buff[4] + buff[5] + buff[6] + buff[7] + buff[8]) % 0xff;
			cout << "[debug] " << px << ' ' << py << ' ' << location.height << ' ' << distance << endl;
			
			//string buff = to_string((long double)1) + ' ' + to_string((long double)px) + ' ' + to_string((long double)py) + '\n';
			//serial.SendData(buff, 10);
			//cout << "[debug] " << strlen(buff) << endl;
			//system("pause");
		}
		else
		{
			char buff[9];
			buff[0] = 0xFF;
			buff[1] = 0xFE;
			buff[2] = 0x00;
			buff[3] = 0x00;
			buff[4] = 0x00;
			buff[5] = 0x00;
			buff[6] = 0x00;
			buff[7] = 0x00;
			buff[8] = 0x00;
			buff[9] = 0x00;
			//serial.SendData(buff, 10);
		}
        if (show_visualization) {
            if(status == 1)
                cv::rectangle(frame, location, cv::Scalar(0, 128, 255), 2);
            cv::imshow("detectracker", frame);
            output_dst << frame;

            char key = cv::waitKey(10);
            if (key == 27 || key == 'q' || key == 'Q')
                break;
        }
    }
    
    time = time / double(cv::getTickFrequency());
    double fps = double(frame_num) / time;
    std::cout << "fps:" << fps << std::endl;
    cv::destroyAllWindows();

	system("pause");
    return 0;
}

