#include "serial.h"
#include <iostream>

using namespace std;


int main(int argc, char * argv[])
{
    string dev = "/dev/ttyUSB0";
    CSerial serial;
    serial.OpenSerialPort(_T("COM6"),115200,8,1);

    string buff = "Hello!";
    serial.SendData(buff.data(), buff.length());


    return 0;
}

