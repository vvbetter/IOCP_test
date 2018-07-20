#pragma once
#include "ThreadPool.h"
#include <map>

enum ClientOPT
{
	REQ_CHECKUID,
	REQ_RECV_CHEKCUID,
};
struct TestParam :public IThreadParam
{
	void* p;
	ClientOPT opt;
	TestParam(void* np,ClientOPT nOpt) :p(np),opt(nOpt) {}
};

#define RECV_UDP_PACKAGE_SIZE 4096

class TestClient
{
public:
	TestClient();
	~TestClient();
public:
	bool InitClient(INT64 uid);
	//����ʵ��
	//У��ID
	bool ReqOpt_CheckUid();
	bool CheckUid();
	//����У��ID����
	bool ReqOpt_RecvCheckUid();
	bool CheckUidRst();
public:
	static void TestFunc(IThreadParam* param);
private:
	void DeleteRecordData(UINT optid);
private:
	INT64 m_uid;
	SOCKET m_socket;
	UINT m_recvID;
	UINT m_sendID;
	map<UINT, char*> m_optRecord; //������Ϣ��¼���ȴ�ȷ�Ϻ�ɾ��
};

