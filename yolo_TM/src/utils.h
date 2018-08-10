#include <vector>
#include <string>
#include <fstream>
#include <numeric>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "template.h"
#include "yolo_v2_class.hpp"

vector<Rect> person_filter(vector<bbox_t> results)
{
    vector<Rect> persons;
    for(size_t i = 0; i < results.size(); i++)
    {
        if( results[i].obj_id == 0 )
        {
            Rect box;
            box.x = results[i].x;
            box.y = results[i].y;
            box.width = results[i].w;
            box.height = results[i].h;
            persons.push_back(box);
        }
    }
    return persons;
}

Rect max_filter(vector<Rect> persons)
{
    int max_aera = persons[0].area();
    int max_index = 0;
    for(size_t i = 1; i < persons.size(); i++)
    {
        int area = persons[i].area();
        if(area > max_aera)
        {
            max_aera = area;
            max_index = i;
        }
    }
    return persons[max_index];
}

void limitRect(cv::Rect &location, cv::Size sz)
{
    cv::Rect window(cv::Point(0, 0), sz);
    location = location & window;
}

string IntToString(int value)
{
    ostringstream convert;
    convert << value;
    return convert.str();
}

string GetCurrentTime()
{
    time_t Time = time(NULL);
    tm* LocalTime = localtime(&Time);
    string Result;
    
    // add the year
    Result += IntToString(LocalTime->tm_year + 1900) + "-";
    // add the month
    Result += IntToString(LocalTime->tm_mon + 1) + "-";
    // add the day
    Result += IntToString(LocalTime->tm_mday) + "_";
    // add the hour
    Result += IntToString(LocalTime->tm_hour) + "-";
    // add the minutes
    Result += IntToString(LocalTime->tm_min) + "-";
    // add the seconds
    Result += IntToString(LocalTime->tm_sec);

    return Result;
}

string GetCurrentTime2()
{
    time_t Time = time(NULL);
    tm* LocalTime = localtime(&Time);
    string Result;
    
    // add the year
    Result += IntToString(LocalTime->tm_year + 1900) + "-";
    // add the month
    Result += IntToString(LocalTime->tm_mon + 1) + "-";
    // add the day
    Result += IntToString(LocalTime->tm_mday) + " ";
    // add the hour
    Result += IntToString(LocalTime->tm_hour) + ":";
    // add the minutes
    Result += IntToString(LocalTime->tm_min) + ":";
    // add the seconds
    Result += IntToString(LocalTime->tm_sec);

    return Result;
}