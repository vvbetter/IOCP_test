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
	NetIoData* pNetData = (NetIoData*)pData;
	USHORT opt_h = pNetData->OPT & 0xffff0000; //高字节为 0：发送数据。1，接收数据
	USHORT opt_l = pNetData->OPT & 0x0000ffff;
	DWORD bytesRecv = 0;
	DWORD flag = 0;

	USHORT msgID = 0;
	USHORT msgLength = 0;
	if (0 == opt_l && 0 == opt_h) //发送了checkuid数据回调
	{
		memset(pNetData->buffer, 0, IOCP_BUFFER_SIZE);
		pNetData->OPT = 0x10000 | 1;
		WSARecv(pNetData->pSocketData->Socket, &pNetData->wsabuf, IOCP_BUFFER_SIZE, &bytesRecv, &flag, &pNetData->overlapped, NULL);
	}
	else if (1 == opt_l && 0 == opt_h)//接收 checkuid数据
	{
		msgID = *(USHORT*)pNetData->buffer;
		msgLength = *(USHORT*)(pNetData->buffer + 2);
		if (Protos_Game60Fishing::ResEnterFishServer == msgID)
		{
			//发送请求进入房间
			Sleep(500);
			ReqJoinTable(pNetData);
		}
		else
		{
			TRANSLOG("接收checkuid数据错误！msgID = %d", msgID);
		}
	}
	else if (2 == opt_l && 0 == opt_h)//发送进入房间回调
	{
		memset(pNetData->buffer, 0, IOCP_BUFFER_SIZE);
		pNetData->OPT = 0x10000 | 2;
		WSARecv(pNetData->pSocketData->Socket, &pNetData->wsabuf, IOCP_BUFFER_SIZE, &bytesRecv, &flag, &pNetData->overlapped, NULL);
	}
	else if (2 == opt_l && 1 == opt_h)//接收进入房间返回
	{
		if (msgID == Protos_Game60Fishing::ResJoinRoom)
		{

		}
		else
		{
			TRANSLOG("接收进入房间消息错误");
		}
	}
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
		TRANSLOG("InetPton error :%d", WSAGetLastError());
		return -1;
	}
	ser_addr.sin_addr = addr;
	int ret = connect(sData->Socket, (sockaddr*)&ser_addr, sizeof(SOCKADDR_IN));
	if (ret == SOCKET_ERROR)
	{
		TRANSLOG("connect error !");
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
		TRANSLOG("uid :%lld checkUid error", uid);
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

bool NetService::ReqJoinTable(NetIoData* pdata)
{
	memset(pdata->buffer, 0, IOCP_BUFFER_SIZE);
	pdata->OPT = 2;
	++pdata->sendID;
	ReqJoinRoomMessage msg;
	msg.set_roomid(0);
	USHORT msgID = Protos_Game60Fishing::ReqJoinRoom;
	memcpy_s(pdata->buffer, 2, &msgID, 2);
	USHORT msgLen = msg.ByteSize();
	memcpy_s(pdata->buffer + 2, 2, &msgLen, 2);
	msg.SerializeToArray(pdata->buffer + 4, msg.ByteSize());
	DWORD bytsSend;
	WSASend(pdata->pSocketData->Socket, &pdata->wsabuf, msgLen + 4, &bytsSend, 0, &pdata->overlapped, NULL);
	return true;
}
