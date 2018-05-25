#include "ColorDetect.h"
#include "serial.h"

int main()
{
	CSerial serial;
	serial.OpenSerialPort(_T("COM6"),115200,8,1);  //打开串口后，自动接收数据
	
	VideoCapture cap(0);
	if(!cap.isOpened())
	{
		cout << "Cannot open web cam..." << endl;
		return -1;
	}

	ColorDetect detector;
	Mat frame;
	Rect box;
	TickMeter tm;
	int framecount = 0;
    while (true)
    {
        cap >> frame;
		if(frame.empty())
		{
			cout << "图片为空..." << endl;
			return -1;
		}
		resize(frame, frame, Size(), 0.5, 0.5);
		framecount++;
		
		tm.start();
		bool ret = detector.detect(frame, box);
		tm.stop();

		if(ret == true)
		{
			short int px = box.x + box.width / 2;
			short int py = box.y + box.height / 2;
			px -= frame.cols / 2;
			py -= frame.rows / 2;
			char buff[9];
			buff[0] = 0xFF;
			buff[1] = 0xFE;
			buff[2] = 0x01;
			buff[3] = (px >> 8) & 0xff;
			buff[4] = px & 0xff;
			buff[5] = (py >> 8) & 0xff;
			buff[6] = py & 0xff;
			buff[7] = 0;
			buff[8] = 0;
			buff[9] = (buff[3] + buff[4] + buff[5] + buff[6] + buff[7] + buff[8]) % 0xff;
			cout << "[debug] px/py/area\t" << px << '/' << py << '/' << box.area() << endl;
			
			//string buff = to_string((long double)1) + ' ' + to_string((long double)px) + ' ' + to_string((long double)py) + '\n';
			serial.SendData(buff, 10);
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
			serial.SendData(buff, 10);
		}

		if(ret == true)
			rectangle(frame, box, Scalar(0, 255, 0), 2, 8);

		resize(frame, frame, Size(), 2, 2);
		imshow("Detector", frame);
		char key = (char) waitKey(10);
		if(key == 27)
				break;
	}

	float costTime = tm.getTimeSec();
	float fps = framecount / costTime;

	cout << "FPS " << fps << endl;

	system("pause");
	return 0;
}