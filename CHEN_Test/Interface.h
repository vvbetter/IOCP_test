#pragma once
#include <WinSock2.h>
#include <sstream>
#include <string>

#define Log(log)  \
	{	stringstream ss; \
		ss << "FILE:" << __FILE__ << " LINE:" << __LINE__ << " : "; \
		ss << log; \
		cout <<ss.str() <<endl; \
	}

#define IOCP_BUFFER_SIZE 8*1024
#define SAFE_DELETE(x)		\
	{						\
		if (x) delete x;	\
		x = NULL;			\
	}						

struct PerIocpData
{
	OVERLAPPED overlapped;
	WSABUF wsabuf;
	char buffer[IOCP_BUFFER_SIZE];
	UINT io_operator;
};

interface IOCPHanlder
{
	virtual const HANDLE GetHanle() = 0;
	virtual bool Callback(PerIocpData* pData) = 0;
	virtual bool RegIOHandle() = 0;
};