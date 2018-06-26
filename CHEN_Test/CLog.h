#pragma once
#include "Interface.h"
#include <string>
using namespace std;

#define TRANSLOG(...) CLog::Instance()->TranceLog(__VA_ARGS__);

class CLog :public IOCPHanlder
{
private:
	CLog();
	virtual ~CLog();
public:
	static CLog * pInstance;
	static CLog* Instance();
public:
	//interface
	virtual const HANDLE GetHanle();
	virtual bool Callback(PerIocpData* pData);
	virtual bool RegIOHandle();
public:
	bool Init();
	bool CloseLog();
	bool TranceLog(const char* str, ...);
private:
	bool m_bOpened;
	string m_logfile;
	HANDLE m_fileHandle;
	PerIocpData* pData;
};