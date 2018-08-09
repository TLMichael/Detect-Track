#include "serial.h"
#include <iostream>

using namespace std;


int main(int argc, char * argv[])
{
    string dev = "/dev/ttyUSB0";
    Serial sel((char *)dev.data());
    sel.setPara();

    string buff = "Hello!";
    sel.writeData(buff.data(), buff.length());

    char buff2[64];
    sel.readData(buff2);

    return 0;
}

