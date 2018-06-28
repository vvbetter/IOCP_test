#pragma once
#include "Interface.h"
#include <list>
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
private:
	list<CreateSocketData> m_sockList;	//创建完成的socket列表
};

