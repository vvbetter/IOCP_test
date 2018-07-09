#pragma once
#include "Interface.h"
#include <map>
#include <process.h>
struct NetIoData :public PerIocpData
{
	CreateSocketData* pSocketData;
	WSABUF wsabuf;
	UINT OPT;
	UINT sendID;
	UINT recvID;
	INT64 uid;
};

class NetService : public IOCPHanlder
{
public:
	NetService();
	~NetService();
public: //interface
	virtual const HANDLE GetHanle() ;
	virtual bool Callback(PerIocpData* pData, DWORD size);
	virtual bool RegIOHandle();
public:
	bool Init();
	SOCKET RegesterNewSocket(INT64 uid, CreateSocketType type, ULONG localIP, USHORT port);
	bool RemoveSocket(SOCKET s);
	//实现功能
	bool RecvCmdData(NetIoData* pdata,DWORD length);
	bool CheckUid(INT64 uid);
	bool ReqJoinTable(INT64 uid);
	bool ReqStartSyncFish(INT64 uid);
private:
	NetIoData * GetIoDataPointByUid(const INT64 uid);
private:
	CRITICAL_SECTION m_cs;
	map<INT64, NetIoData*> m_IoDataMap;	//创建完成的socket列表
};

