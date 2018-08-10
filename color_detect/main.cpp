#include "ColorDetect.h"

int main()
{
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
			cout << "Í¼Æ¬Îª¿Õ..." << endl;
			return -1;
		}
		resize(frame, frame, Size(), 0.5, 0.5);
		framecount++;
		
		tm.start();
		bool ret = detector.detect(frame, box);
		tm.stop();

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