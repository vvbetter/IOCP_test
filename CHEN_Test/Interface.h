#pragma once
#include <WinSock2.h>
#include <sstream>
#include <string>
#include <iostream>

using namespace std;

#define Log(log)  \
	{	stringstream ss; \
		ss << "FILE:" << __FILE__ << " LINE:" << __LINE__ << " : "; \
		ss << log; \
		cout <<ss.str() <<endl; \
	}
#define IOCP_BUFFER_SIZE 8*1024
#define SAFE_DELETE(x)		\
	{						\
		if (x)				\
		{delete x;			\
		x = NULL;}			\
	}						

struct PerIocpData
{
	OVERLAPPED overlapped;
	char buffer[IOCP_BUFFER_SIZE];
};

interface IOCPHanlder
{
	virtual const HANDLE GetHanle() = 0;
	virtual bool Callback(PerIocpData* pData, DWORD size) = 0;
	virtual bool RegIOHandle() = 0;
};

struct CreateSocketData
{
	SOCKET			Socket;
	USHORT			Port;
	SOCKADDR_IN		Addr;
	bool			RegState;
};

enum CreateSocketType
{
	CST_UDP_IOCP = 0x1,
	CST_TCP_IOCP = 0x2,
	CST_UDP = 0x4,
	CST_TCP = 0x8,
};

extern bool CreateSocket(CreateSocketType type, UINT localIP, USHORT port, CreateSocketData &csd);