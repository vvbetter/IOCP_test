#include "stdafx.h"
#include "TestClient.h"
#include "proto_source/Game60_FishingMessage.pb.h"
#include <ws2tcpip.h>

using namespace com::game::proto;

TestClient::TestClient()
{
	m_recvID = 1;
	m_sendID = 1;
}

TestClient::~TestClient()
{
}
#define  UDP_SOCKET_PORT_START 50000
bool TestClient::InitClient(INT64 uid)
{
	CreateSocketData*  sData = new CreateSocketData();
	if (CreateSocket(CST_UDP, 0, 0, *sData) == false)
	{
		Log("NetService Create New Socket Error");
		return false;
	}
	SOCKADDR_IN ser_addr;
	ser_addr.sin_port = htons(UDP_SOCKET_PORT_START + static_cast<USHORT>(uid));
	ser_addr.sin_family = AF_INET;

	IN_ADDR addr;
	int d = InetPton(AF_INET, L"192.168.0.89", &addr);
	if (1 != d)
	{
		closesocket(sData->Socket);
		Log("InetPton error :%d", WSAGetLastError());
		return false;
	}
	ser_addr.sin_addr = addr;
	int ret = connect(sData->Socket, (sockaddr*)&ser_addr, sizeof(SOCKADDR_IN));
	if (ret == SOCKET_ERROR)
	{
		Log("connect error !");
		closesocket(sData->Socket);
		return false;
	}
	m_uid = uid;
	m_socket = sData->Socket;
	SAFE_DELETE(sData);
	return true;
}

bool TestClient::ReqOpt_CheckUid()
{
	_ThreadElem* nElem = new _ThreadElem(TestClient::TestFunc, new TestParam(this, REQ_CHECKUID), 1, 0, 0);
	bool ret = ThreadPool::Instance()->AddElem(nElem);
	return ret;
}

bool TestClient::CheckUid()
{
	ReqEnterFishServerMessage msg;
	msg.set_playeronlyid(m_uid);
	USHORT msgID = Protos_Game60Fishing::ReqEnterFishServer;
	USHORT msgLen = msg.ByteSize();
	char* pBuff = new char[4 + msgLen];
	memcpy_s(pBuff, 2, &msgID, 2);
	memcpy_s(pBuff + 2, 2, &msgLen, 2);
	msg.SerializeToArray(pBuff + 4, msg.ByteSize());
	int ret = send(m_socket, pBuff, msgLen + 4, 0);
	if (ret == SOCKET_ERROR)
	{
		Log("uid " << m_uid << "Checkuid send error,%d", WSAGetLastError());
	}
	m_optRecord[0] = pBuff;
	ReqOpt_RecvCheckUid();
	return true;
}

bool TestClient::ReqOpt_RecvCheckUid()
{
	_ThreadElem* nElem = new _ThreadElem(TestClient::TestFunc, new TestParam(this, REQ_RECV_CHEKCUID), 1, 0, 0);
	bool ret = ThreadPool::Instance()->AddElem(nElem);
	return ret;
}

bool TestClient::CheckUidRst()
{
	bool ret = true;
	char buff[RECV_UDP_PACKAGE_SIZE] = { 0 };
	int size = recv(m_socket, buff, RECV_UDP_PACKAGE_SIZE, 0);
	if (size == SOCKET_ERROR)
	{
		Log("uid " << m_uid << " recv check uid data size = -1,error code:"<< WSAGetLastError());
		ret = false;
	}
	DeleteRecordData(0);
	return ret;
}

void TestClient::TestFunc(IThreadParam * param)
{
	TestParam* pt = (TestParam*)param;
	TestClient* client = (TestClient*)pt->p;
	switch (pt->opt)
	{
	case REQ_CHECKUID:
		client->CheckUid();
	case REQ_RECV_CHEKCUID:
		client->CheckUidRst();
	default:
		break;
	}
}

void TestClient::DeleteRecordData(UINT optid)
{
	auto it = m_optRecord.find(optid);
	if (it != m_optRecord.end())
	{
		SAFE_DELETE(it->second);
		m_optRecord.erase(it);
	}
}
