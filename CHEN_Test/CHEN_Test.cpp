// CHEN_Test.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "IOCPHelper.h"
#include "CLog.h"
#include "NetService.h"
#include <iostream>
#include <fstream>
#include <ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

int main()
{
	IOCPHelper::Instance()->Init();
	CLog::Instance()->Init();
	NetService service;
	service.Init();

	IN_ADDR addr;
	int d = InetPton(AF_INET, L"192.168.0.89", &addr);
	if (1 != d)
	{
		cout << "InetPton error " << WSAGetLastError() << endl;
		return -1;
	}
	SOCKET s = service.RegesterNewSocket(1, CST_UDP_IOCP, 0, 0);
	if (s == INVALID_SOCKET)
	{
		cout << "RegesterNewSocket error" << endl;
		return -1;
	}
	service.CheckUid(1);
	Sleep(100000);
	service.RemoveSocket(s);
    return 0;
}

