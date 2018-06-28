#include "stdafx.h"
#include "NetService.h"
#include "IOCPHelper.h"
#include "CLog.h"

NetService::NetService()
{
}


NetService::~NetService()
{
	WSACleanup();
}

const HANDLE NetService::GetHanle()
{
	for (auto it = m_sockList.rbegin(); it != m_sockList.rend(); ++it)
	{
		if (it->RegState == false)
		{
			it->RegState = true;
			return (HANDLE)it->Socket;
		}
	}
	TRANSLOG("NetService Get Unkown Handle");
	return INVALID_HANDLE_VALUE;
}

bool NetService::Callback(PerIocpData * pData)
{
	return false;
}

bool NetService::RegIOHandle()
{
	IOCPHelper::Instance()->RegHandle(this);
	return true;
}

bool NetService::Init()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		TRANSLOG("WSAStartup 初始化失败:%d", WSAGetLastError());
		return false;
	}
	return true;
}

SOCKET NetService::RegesterNewSocket(CreateSocketType type, ULONG localIP, USHORT port)
{
	CreateSocketData sData;
	if (CreateSocket(type, localIP, port, sData) == false)
	{
		TRANSLOG("NetService Create New Socket Error");
		return INVALID_SOCKET;
	}
	//TODO 多线程加锁
	m_sockList.push_back(sData);
	RegIOHandle();
	return sData.Socket;
}

bool NetService::RemoveSocket(SOCKET s)
{
	for (auto it = m_sockList.begin(); it != m_sockList.end(); ++it)
	{
		if (it->Socket == s)
		{
			closesocket(s);
			it = m_sockList.erase(it);
			return true;
		}
	}
	return false;
}
