//
// Created by michael on 17-12-15.
//

#include "serial.h"

/**
 * @brief   构造函数，初始化文件句柄
 * @param   dev 类型 char*    串口号
 */
Serial::Serial(char *dev)
{
    fd = open( dev, O_RDWR | O_NOCTTY );         //| O_NOCTTY | O_NDELAY
    if (-1 == fd)
    {
        perror("Can't Open Serial Port");
        exit(0);
    }
}

Serial::~Serial()
{
    close(fd);
}

/**
 * @brief   延迟
 * @param   sec     类型 int  延迟秒数
 * @return  void
 */
void Serial::delay(int sec)
{
    time_t start_time, cur_time;
    time(&start_time);
    do{
        time(&cur_time);
    }while((cur_time - start_time) < sec);
}

/**
 * @brief   设置串口通信速率,数据位，停止位和效验位
 * @param   speed  类型 int  串口速度
 * @param   databits 类型  int 数据位   取值为 7 或者8
 * @oaran   stopbits 类型  int 停止位   取值为 1 或者2
 * @param   parity  类型  int  效验类型 取值为N,E,O,S
 * @return  bool
 */
bool Serial::setPara(int speed, int databits, int stopbits, int parity)
{
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if  (status != 0)
            {
                perror("tcsetattr fd1");
                return false;
            }
            tcflush(fd,TCIOFLUSH);
        }
    }

    struct termios options;
    options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
    options.c_oflag  &= ~OPOST;   /*Output*/
    if  ( tcgetattr( fd,&options)  !=  0)
    {
        perror("SetupSerial 1");
        return false;
    }
    options.c_cflag &= ~CSIZE;
    switch (databits) /*设置数据位数*/
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr,"Unsupported data size/n"); return false;
    }
    switch (parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;   /* Clear parity enable */
            options.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
            options.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;     /* Enable parity */
            options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
            options.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
        case 'S':
        case 's':  /*as no parity*/
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;break;
        default:
            fprintf(stderr,"Unsupported parity/n");
            return false;
    }
    /* 设置停止位*/
    switch (stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr,"Unsupported stop bits/n");
            return false;
    }
    /* Set input parity option */
    if (parity != 'n')
        options.c_iflag |= INPCK;
    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
        perror("SetupSerial 3");
        return false;
    }
    return true;
}

/**
 * @brief   设置串口通信速率/波特率
 * @param   speed  类型 int  串口速度/波特率
 * @return  bool
 */
bool Serial::setBaudRate(int speed)
{
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if  (speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if  (status != 0)
            {
                perror("tcsetattr fd1");
                return false;
            }
            tcflush(fd,TCIOFLUSH);
        }
    }
    return true;
}

/**
 * @brief   写数据
 * @param   data        类型 const char *     字符串
 * @param   datalength  类型  int             字符串长度
 * @return  int 写入字符串的长度
 */
int Serial::writeData(const char *data, int datalength)
{
    int nwrite;
    nwrite = write(fd, data, datalength);
    return nwrite;
}

/**
 * @brief   读数据
 * @param   data        类型  char *     字符串
 * @param   datalength  类型  int       字符串长度
 * @return  int 读出字符串的长度
 */
int Serial::readData(char *data, int datalength)
{
    int nread;
    nread = read(fd, data, 5);
    data[nread] = '\0';
    return nread;
}