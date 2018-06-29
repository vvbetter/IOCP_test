#include "stdafx.h"
#include "NetService.h"
#include "IOCPHelper.h"
#include "CLog.h"
#include "CS_LockGuide.h"
#include "proto_source/Game60_FishingMessage.pb.h"

using namespace com::game::proto;

NetService::NetService()
{
}


NetService::~NetService()
{
	WSACleanup();
	DeleteCriticalSection(&m_cs);
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
		TRANSLOG("WSAStartup ��ʼ��ʧ��:%d", WSAGetLastError());
		return false;
	}
	InitializeCriticalSection(&m_cs);
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
	{
		AUTO_LOCKER(m_cs);
		m_sockList.push_back(sData);
	}
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
			{
				AUTO_LOCKER(m_cs);
				it = m_sockList.erase(it);
			}
			return true;
		}
	}
	return false;
}
struct NetIoData :public PerIocpData
{
	WSABUF wsabuf;
	UINT operatro;
};

bool NetService::CheckUid(INT64 uid)
{
	ReqEnterFishServerMessage msg;
	msg.set_playeronlyid(uid);
	return false;
}
