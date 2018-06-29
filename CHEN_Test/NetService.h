#pragma once
#include "Interface.h"
#include <list>
#include <process.h>
class NetService : public IOCPHanlder
{
public:
	NetService();
	~NetService();
public: //interface
	virtual const HANDLE GetHanle() ;
	virtual bool Callback(PerIocpData* pData);
	virtual bool RegIOHandle();
public:
	bool Init();
	SOCKET RegesterNewSocket(CreateSocketType type, ULONG localIP, USHORT port);
	bool RemoveSocket(SOCKET s);
	//ʵ�ֹ���
	bool CheckUid(INT64 uid);
private:
	CRITICAL_SECTION m_cs;
	list<CreateSocketData> m_sockList;	//������ɵ�socket�б�
};

