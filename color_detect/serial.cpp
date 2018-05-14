#include "serial.h"
#include <process.h>

typedef unsigned (__stdcall *PTHREAD_START) (void *);

CSerial::CSerial(void)
{
	m_hComm = INVALID_HANDLE_VALUE;
}

CSerial::~CSerial(void)
{
	if(m_hComm != INVALID_HANDLE_VALUE){
		CloseHandle(m_hComm);
	}
}

/*********************************************************************************************
 * 功能    ：	读串口线程回调函数 
 * 描述	   ：	收到数据后，简单的显示出来
 ********************************************************************************************/
DWORD WINAPI CommProc(LPVOID lpParam){

	CSerial* pSerial = (CSerial*)lpParam;  //

	//清空串口
	PurgeComm(pSerial->m_hComm,PURGE_RXCLEAR|PURGE_TXCLEAR);

	char buf[512];
	DWORD dwRead;
	while(pSerial->m_hComm != INVALID_HANDLE_VALUE){
		BOOL bReadOK = ReadFile(pSerial->m_hComm,buf,512,&dwRead,NULL);
		if(bReadOK && (dwRead > 0)){
			buf[dwRead] = '\0';
			cout << "串口收到数据：" << buf << endl;
		}
	}
	return 0;
}


/*******************************************************************************************
 * 功能     ：	打开串口
 * port     :	串口号, 如_T("COM1:")
 * baud_rate:	波特率
 * date_bits:	数据位（有效范围4~8）
 * stop_bit :	停止位
 * parity   :	奇偶校验。默认为无校验。NOPARITY 0； ODDPARITY 1；EVENPARITY 2；MARKPARITY 3；SPACEPARITY 4
 ********************************************************************************************/
BOOL CSerial::OpenSerialPort(TCHAR* port,UINT baud_rate,BYTE date_bits,BYTE stop_bit,BYTE parity){
	//打开串口
	m_hComm = CreateFile(port,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);//独占方式打开串口

	if(m_hComm == INVALID_HANDLE_VALUE){
		cout << "打开串口失败，请查看该串口是否已被占用" << endl;
		//_stprintf(err,_T("打开串口%s 失败，请查看该串口是否已被占用"),port);
		//MessageBox(NULL,err,_T("提示"),MB_OK);
		return FALSE;
	}	

	//MessageBox(NULL,_T("打开成功"),_T("提示"),MB_OK);

	//获取串口默认配置
	DCB dcb;
	if(!GetCommState(m_hComm,&dcb)){
		cout << "获取串口当前属性参数失败" << endl;
		//MessageBox(NULL,_T("获取串口当前属性参数失败"),_T("提示"),MB_OK);
	}

	//配置串口参数
	dcb.BaudRate = baud_rate;	//波特率
	dcb.fBinary = TRUE;			//二进制模式。必须为TRUE
	dcb.ByteSize = date_bits;	//数据位。范围4-8
	dcb.StopBits = ONESTOPBIT;	//停止位

	if(parity == NOPARITY){
		dcb.fParity = FALSE;	//奇偶校验。无奇偶校验
		dcb.Parity = parity;	//校验模式。无奇偶校验
	}else{
		dcb.fParity = TRUE;		//奇偶校验。
		dcb.Parity = parity;	//校验模式。无奇偶校验
	}

	dcb.fOutxCtsFlow = FALSE;	//CTS线上的硬件握手
	dcb.fOutxDsrFlow = FALSE;	//DST线上的硬件握手
	dcb.fDtrControl = DTR_CONTROL_ENABLE; //DTR控制
	dcb.fDsrSensitivity = FALSE;
	dcb.fTXContinueOnXoff = FALSE;//
	dcb.fOutX = FALSE;			//是否使用XON/XOFF协议
	dcb.fInX = FALSE;			//是否使用XON/XOFF协议
	dcb.fErrorChar = FALSE;		//是否使用发送错误协议
	dcb.fNull = FALSE;			//停用null stripping
	dcb.fRtsControl = RTS_CONTROL_ENABLE;//
	dcb.fAbortOnError = FALSE;	//串口发送错误，并不终止串口读写

	//设置串口参数
	if (!SetCommState(m_hComm,&dcb)){
		cout << "设置串口参数失败" << endl;
		//MessageBox(NULL,_T("设置串口参数失败"),_T("提示"),MB_OK);
		return FALSE;
	}

	//设置串口事件
	SetCommMask(m_hComm,EV_RXCHAR); //在缓存中有字符时产生事件
	SetupComm(m_hComm,16384,16384);

	//设置串口读写时间
	COMMTIMEOUTS CommTimeOuts;
	GetCommTimeouts(m_hComm,&CommTimeOuts);
	CommTimeOuts.ReadIntervalTimeout = MAXDWORD;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 10;
	CommTimeOuts.WriteTotalTimeoutConstant = 1000;

	if(!SetCommTimeouts(m_hComm,&CommTimeOuts)){
		cout << "设置串口时间失败" << endl;
		//MessageBox(NULL,_T("设置串口时间失败"),_T("提示"),MB_OK);
		return FALSE;
	}

	//创建线程，读取数据
	HANDLE hReadCommThread = (HANDLE) _beginthreadex(NULL,0,(PTHREAD_START) CommProc,(LPVOID) this,0,NULL);

	return TRUE;
}

/********************************************************************************************
 * 功能    ：	通过串口发送一条数据
 ********************************************************************************************/
BOOL CSerial::SendData(const char* data,int len){
	if(m_hComm == INVALID_HANDLE_VALUE){
		cout << "串口未打开" << endl;
		//MessageBox(NULL,_T("串口未打开"),_T("提示"),MB_OK);
		return FALSE;
	}

	//清空串口
	PurgeComm(m_hComm,PURGE_RXCLEAR|PURGE_TXCLEAR);

	//写串口
	DWORD dwWrite = 0;
	DWORD dwRet = WriteFile(m_hComm,data,len,&dwWrite,NULL);

	//清空串口
	PurgeComm(m_hComm,PURGE_RXCLEAR|PURGE_TXCLEAR);

	if(!dwRet){
		cout << "串口未打开" << endl;
		//MessageBox(NULL,_T("发送数据失败"),_T("提示"),MB_OK);
		return FALSE;
	}
	return TRUE;
}