// CHEN_Test.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "IOCPHelper.h"
#include "CLog.h"
#include "NetService.h"
#include "ThreadPool.h"
#include "TestClient.h"
#include <iostream>
#include <fstream>
#include <ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;

int main()
{
	IOCPHelper::Instance()->Init();
	CLog::Instance()->Init();
	ThreadPool::Instance()->Init(10);

	TestClient* pt = new TestClient();
	_ThreadElem* pElem = new _ThreadElem(TestClient::TestFunc, new TestParam(pt),10, 0, 100);
	ThreadPool::Instance()->AddElem(pElem);


//	NetService service;
//	service.Init();
//
//	IN_ADDR addr;
//	int d = InetPton(AF_INET, L"192.168.0.89", &addr);
//	if (1 != d)
//	{
//		cout << "InetPton error " << WSAGetLastError() << endl;
//		return -1;
//	}
//#define PLAYER_NUMBER 3
//	for (INT64 i = 1; i <= PLAYER_NUMBER; ++i)
//	{
//		SOCKET s = service.RegesterNewSocket(i, CST_UDP_IOCP, 0, 0);
//		if (s == INVALID_SOCKET)
//		{
//			cout << "RegesterNewSocket error" << endl;
//			return -1;
//		}
//		service.CheckUid(i);
//	}
//	Sleep(2000);
//	for (INT64 i = 1; i <= PLAYER_NUMBER; ++i)
//	{
//		service.ReqJoinTable(i);
//	}
//	Sleep(5000);
//	for (INT64 i = 1; i <= PLAYER_NUMBER; ++i)
//	{
//		service.ReqStartSyncFish(i);
//	}
//	Sleep(5000);
//	while (1)
//	{
//		for (INT64 i = 1; i <= PLAYER_NUMBER; ++i)
//		{
//			service.ReqBullet(i);
//		}
//		Sleep(1000);
//	}

	Sleep(10000000);
	//service.RemoveSocket(s);
    return 0;
}

