#include "stdafx.h"
#include "NetService.h"
#include "IOCPHelper.h"
#include "CLog.h"
#include "CS_LockGuide.h"
#include "proto_source/Game60_FishingMessage.pb.h"
#include <ws2tcpip.h>

using namespace com::game::proto;
#define SET_ANSWER(X) ((X) | 0x80000000)
#define IS_ANSWER(X) ((X & 0x80000000) != 0)
#define RESET_ANSWER(X) ((X) &= 0x7fffffff)
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
	TRANSLOG("NetService Get Unkown Handle\n");
	return INVALID_HANDLE_VALUE;
}

bool NetService::Callback(PerIocpData * pData, DWORD size)
{
	NetIoData* pNetData = (NetIoData*)pData;
	USHORT opt_h = ((pNetData->OPT >> 16) & 0xffff); //���ֽ�Ϊ 0���������ݡ�1����������
	USHORT opt_l = pNetData->OPT & 0xffff;
	DWORD bytesRecv = 0;
	DWORD flag = 0;

	if (1 == opt_l && 0 == opt_h) //������checkuid���ݻص�
	{
		TRANSLOG("checkuid �ص�\n");
		memset(pNetData->buffer, 0, IOCP_BUFFER_SIZE);
		pNetData->OPT = 0x10000 | 1;
		pNetData->wsabuf.len = IOCP_BUFFER_SIZE;
		int ret = WSARecv(pNetData->pSocketData->Socket, &pNetData->wsabuf, 1, &bytesRecv, &flag, &pNetData->overlapped, NULL);
		if (ret == SOCKET_ERROR)
		{
			int e_code = WSAGetLastError();
			if (e_code != WSA_IO_PENDING)
			{
				TRANSLOG("checkuid���ݻص� WSARecv ����%d\n", e_code);
			}
		}
		else
		{
			TRANSLOG("checkuid �ص��ɹ�\n");
		}
	}
	else if (1 == opt_h && 1 == opt_l )//���� checkuid����
	{
		TRANSLOG("���� checkuid ��������\n");
		USHORT msgID = 0;
		USHORT msgLength = 0;
		msgID = *(USHORT*)pNetData->buffer;
		msgLength = *(USHORT*)(pNetData->buffer + 2);
		if (Protos_Game60Fishing::ResEnterFishServer == msgID)
		{
			//���ܷ�����׼�������Ϣ
			TRANSLOG("��֤���IDͨ��\n");
		}
		else
		{
			TRANSLOG("����checkuid���ݴ���msgID = %d,size = %d\n", msgID, size);
		}
	}
	else if (0 == opt_h && 2 == opt_l)//���ͽ��뷿��ص�
	{
		TRANSLOG("���ͽ��뷿��ص�\n");
		memset(pNetData->buffer, 0, IOCP_BUFFER_SIZE);
		pNetData->OPT = 0xf0000 | 0;
		pNetData->wsabuf.len = IOCP_BUFFER_SIZE;
		WSARecv(pNetData->pSocketData->Socket, &pNetData->wsabuf, 1, &bytesRecv, &flag, &pNetData->overlapped, NULL);
	}
	else if (0 == opt_h && 3 == opt_l) //���ͳ������������Ϣ �ص�
	{
		TRANSLOG("���ͳ������������Ϣ�ص�\n");
		memset(pNetData->buffer, 0, IOCP_BUFFER_SIZE);
		pNetData->OPT = 0xf0000 | 0;
		pNetData->wsabuf.len = IOCP_BUFFER_SIZE;
		WSARecv(pNetData->pSocketData->Socket, &pNetData->wsabuf, 1, &bytesRecv, &flag, &pNetData->overlapped, NULL);
	}
	else if (2 == opt_h && 0 == opt_l)//���ͽ��յ��ķ�����������ϢID �ص�
	{
		TRANSLOG("���ͽ��յ��ķ�����������ϢID �ص�\n");
		//����������Ϣ
		memset(pNetData->buffer, 0, IOCP_BUFFER_SIZE);
		pNetData->OPT = 0xf0000 | 0;
		pNetData->wsabuf.len = IOCP_BUFFER_SIZE;
		WSARecv(pNetData->pSocketData->Socket, &pNetData->wsabuf, 1, &bytesRecv, &flag, &pNetData->overlapped, NULL);
	}
	else if (0xf == opt_h && 0 == opt_l) //���ܷ���������
	{
		TRANSLOG("���շ���������\n");
		RecvCmdData(pNetData, size);
	}
	else
	{
		TRANSLOG("opt_h = %d,opt_l = %d\n", opt_h, opt_l);
		RecvCmdData(pNetData, size);
	}
	//TRANSLOG("opt_h = %d,opt_l = %d\n", opt_h, opt_l);
	return true;
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
	ser_addr.sin_port = htons(UDP_SOCKET_PORT_START + static_cast<USHORT>(uid));
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
	pIodata->recvID = 1;
	pIodata->sendID = 1;
	pIodata->uid = uid;
	pIodata->wsabuf.buf = pIodata->buffer;
	pIodata->wsabuf.len = IOCP_BUFFER_SIZE;
	pIodata->overlapped.hEvent = WSACreateEvent();
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
bool NetService::RecvCmdData(NetIoData * pdata, DWORD length)
{
	if (length < 4)
	{
		TRANSLOG("uid:%lld RecvCmdData error size = %d\n", pdata->uid, length);
	}
	else if (length == 4)
	{
		UINT anserID = *(UINT*)pdata->buffer;
	}
	else
	{
		char* pBuff = pdata->buffer;
		char* pend = pBuff + length;
		UINT anserID = *(UINT*)pBuff;
		anserID = RESET_ANSWER(anserID);
		pBuff += 4;
		while (pBuff != pend)
		{
			UINT serSendID = *(UINT*)pBuff;
			pBuff += 4;
			USHORT msgID = *(USHORT*)pBuff;
			pBuff += 2;
			USHORT msgLength = *(USHORT*)pBuff;
			pBuff += (msgLength + 2);
			pdata->recvID = (serSendID == pdata->recvID + 1 ? serSendID : pdata->recvID);
			TRANSLOG("uid:%lld �յ����� server answerid:%d, server sendid:%d, msgID:%d, msglength:%d\n", pdata->uid, anserID, serSendID, msgID, msgLength);
		}
	}
	//�ظ��������յ���ϢID
	memset(pdata->buffer, 0, IOCP_BUFFER_SIZE);
	DWORD bytsSend;
	UINT rcid = pdata->recvID;
	UINT backAnswer = SET_ANSWER(rcid);
	UINT tb = htonl(backAnswer);
	memcpy_s(pdata->buffer, 4, &tb, 4);
	pdata->OPT = 0x20000 | 0;
	pdata->wsabuf.len = 4;
	TRANSLOG("uid:%lld ���� RecvCmdData length = %d, �ظ���������Ϣid = %d\n", pdata->uid, length, pdata->recvID);
	for (UINT i = 0; i < 4; ++i)
	{
		TRANSLOG("%d ", (int)*((char*)pdata->buffer + i));
	}
	int ret = WSASend(pdata->pSocketData->Socket, &pdata->wsabuf, 1, &bytsSend, 0, &pdata->overlapped, NULL);
	if (ret == SOCKET_ERROR)
	{
		TRANSLOG("�ظ���������ϢID���� %d", WSAGetLastError());
	}
	return true;
}

bool NetService::CheckUid(INT64 uid)
{
	NetIoData* pdata = GetIoDataPointByUid(uid);
	if (pdata == NULL) return false;
	pdata->OPT = 1;
	ReqEnterFishServerMessage msg;
	msg.set_playeronlyid(uid);
	CHAR* pBuff = pdata->buffer;
	USHORT msgID = Protos_Game60Fishing::ReqEnterFishServer;
	memcpy_s(pBuff, 2, &msgID, 2);
	USHORT msgLen = msg.ByteSize();
	memcpy_s(pBuff + 2, 2, &msgLen, 2);
	msg.SerializeToArray(pBuff + 4, msg.ByteSize());
	DWORD bytsSend;
	pdata->wsabuf.len = msgLen + 4;
	int ret = WSASend(pdata->pSocketData->Socket, &pdata->wsabuf, 1, &bytsSend, 0, &pdata->overlapped, NULL);
	if (ret == SOCKET_ERROR)
	{
		TRANSLOG("Checkuid send error,%d", WSAGetLastError());
	}
	return true;
}

bool NetService::ReqJoinTable(INT64 uid)
{
	NetIoData* pdata = GetIoDataPointByUid(uid);
	if (pdata == NULL) return false;
	memset(pdata->buffer, 0, IOCP_BUFFER_SIZE);
	pdata->OPT = 2;
	++pdata->sendID;
	ReqJoinRoomMessage msg;
	msg.set_roomid(0);
	UINT answerID = SET_ANSWER(pdata->recvID);
	UINT ta = htonl(answerID);
	memcpy_s(pdata->buffer, 4, &ta, 4);
	UINT sendID = htonl(pdata->sendID);
	memcpy_s(pdata->buffer + 4, 4, &sendID, 4);
	USHORT msgID = htons(Protos_Game60Fishing::ReqJoinRoom);
	memcpy_s(pdata->buffer + 8, 2, &msgID, 2);
	USHORT msgLen = htons(msg.ByteSize());
	memcpy_s(pdata->buffer + 10, 2, &msgLen, 2);
	msg.SerializeToArray(pdata->buffer + 12, msg.ByteSize());
	DWORD bytsSend;
	pdata->wsabuf.len = msg.ByteSize() + 12;
	int ret = WSASend(pdata->pSocketData->Socket, &pdata->wsabuf, 1, &bytsSend, 0, &pdata->overlapped, NULL);
	if (ret == SOCKET_ERROR)
	{
		TRANSLOG("ReqJoinTable send error,%d", WSAGetLastError());
	}
	return true;
}

bool NetService::ReqStartSyncFish(INT64 uid)
{
	NetIoData* pdata = GetIoDataPointByUid(uid);
	memset(pdata->buffer, 0, IOCP_BUFFER_SIZE);
	pdata->OPT = 3;
	++pdata->sendID;
	ReqStartSyncFishMessage msg;
	UINT answerID = SET_ANSWER(pdata->recvID);
	UINT tc = htonl(answerID);
	memcpy_s(pdata->buffer, 4, &tc, 4);
	UINT sendID = htonl(pdata->sendID);
	memcpy_s(pdata->buffer + 4, 4, &sendID, 4);
	USHORT msgID = htons(Protos_Game60Fishing::ReqStartSyncFish);
	memcpy_s(pdata->buffer + 8, 2, &msgID, 2);
	USHORT msgLen = htons(msg.ByteSize());
	memcpy_s(pdata->buffer + 10, 2, &msgLen, 2);
	msg.SerializeToArray(pdata->buffer + 12, msg.ByteSize());
	DWORD bytsSend;
	pdata->wsabuf.len = msg.ByteSize() + 12;
	int ret = WSASend(pdata->pSocketData->Socket, &pdata->wsabuf, 1, &bytsSend, 0, &pdata->overlapped, NULL);
	if (ret == SOCKET_ERROR)
	{
		TRANSLOG("ReqStartSyncFish send error,%d", WSAGetLastError());
	}
	return true;
}

NetIoData * NetService::GetIoDataPointByUid(const INT64 uid)
{
	AUTO_LOCKER(m_cs);
	auto uit = m_IoDataMap.find(uid);
	if (uit == m_IoDataMap.end())
	{
		return NULL;
	}
	return uit->second;
}
