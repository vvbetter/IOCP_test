#include "stdafx.h"
#include "NetService.h"
#include "IOCPHelper.h"
#include "CLog.h"
#include "CS_LockGuide.h"
#include "proto_source/Game60_FishingMessage.pb.h"
#include <ws2tcpip.h>

using namespace com::game::proto;
#define SET_ANSWER(X) ((X) | 0x80000000)

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
	for (auto it = m_IoDataMap.rbegin(); it != m_IoDataMap.rend(); ++it)
	{
		if (it->second->pSocketData->RegState == false)
		{
			it->second->pSocketData->RegState = true;
			return (HANDLE)it->second->pSocketData->Socket;
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
		TRANSLOG("WSAStartup ³õÊ¼»¯Ê§°Ü:%d", WSAGetLastError());
		return false;
	}
	InitializeCriticalSection(&m_cs);
	return true;
}
#define  UDP_SOCKET_PORT_START 50000
SOCKET NetService::RegesterNewSocket(INT64 uid, CreateSocketType type, ULONG localIP, USHORT port)
{
	CreateSocketData*  sData = new CreateSocketData();
	if (CreateSocket(type, localIP, port, *sData) == false)
	{
		TRANSLOG("NetService Create New Socket Error");
		return INVALID_SOCKET;
	}
	SOCKADDR_IN ser_addr;
	ser_addr.sin_port = htons(UDP_SOCKET_PORT_START + uid);
	ser_addr.sin_family = AF_INET;

	IN_ADDR addr;
	int d = InetPton(AF_INET, L"192.168.0.89", &addr);
	if (1 != d)
	{
		closesocket(sData->Socket);
		cout << "InetPton error " << WSAGetLastError() << endl;
		return -1;
	}
	ser_addr.sin_addr = addr;
	int ret = connect(sData->Socket, (sockaddr*)&ser_addr, sizeof(SOCKADDR_IN));
	if (ret == SOCKET_ERROR)
	{
		cout << "connect error !" << endl;
		closesocket(sData->Socket);
		return INVALID_SOCKET;
	}
	NetIoData* pIodata = new NetIoData();
	memset(pIodata, 0, sizeof(NetIoData));
	pIodata->pSocketData = sData;
	pIodata->recvID = 0;
	pIodata->sendID = 0;
	pIodata->wsabuf.buf = pIodata->buffer;
	pIodata->wsabuf.len = IOCP_BUFFER_SIZE;
	{
		AUTO_LOCKER(m_cs);
		m_IoDataMap[uid] = pIodata;
	}
	RegIOHandle();
	return sData->Socket;
}

bool NetService::RemoveSocket(SOCKET s)
{
	//for (auto it = m_sockMap.begin(); it != m_sockMap.end(); ++it)
	//{
	//	if (it->second.Socket == s)
	//	{
	//		closesocket(s);
	//		{
	//			AUTO_LOCKER(m_cs);
	//			it = m_sockMap.erase(it);
	//		}
	//		return true;
	//	}
	//}
	return false;
}

bool NetService::CheckUid(INT64 uid)
{
	auto it = m_IoDataMap.find(uid);
	if (it == m_IoDataMap.end())
	{
		cout << "uid :" << uid << "checkUid error" << endl;
		return false;
	}
	it->second->OPT = 1;
	ReqEnterFishServerMessage msg;
	msg.set_playeronlyid(uid);
	CHAR* pBuff = it->second->buffer;
	USHORT msgID = Protos_Game60Fishing::ReqEnterFishServer;
	memcpy_s(pBuff, 2, &msgID, 2);
	USHORT msgLen = msg.ByteSize();
	memcpy_s(pBuff + 2, 2, &msgLen, 2);
	msg.SerializeToArray(pBuff + 4, msg.ByteSize());
	DWORD bytsSend;
	WSASend(it->second->pSocketData->Socket, &it->second->wsabuf, msgLen + 4, &bytsSend, 0, &it->second->overlapped, NULL);
	return true;
}
