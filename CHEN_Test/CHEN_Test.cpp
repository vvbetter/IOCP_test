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
	for (INT64 i = 1; i < 2; ++i)
	{
		SOCKET s = service.RegesterNewSocket(1, CST_UDP_IOCP, 0, 0);
		if (s == INVALID_SOCKET)
		{
			cout << "RegesterNewSocket error" << endl;
			return -1;
		}
		service.CheckUid(i);
		Sleep(2000);
		service.ReqJoinTable(i);
		Sleep(5000);
		service.ReqStartSyncFish(i);
	}
	Sleep(10000000);
	//service.RemoveSocket(s);
    return 0;
}

