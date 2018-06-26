#pragma once
#include <iostream>
#include "Interface.h"
#pragma comment(lib,"Ws2_32.lib")

using namespace std;

class IOCPHelper
{
private:
	IOCPHelper();
	~IOCPHelper();
public:
	static IOCPHelper* pInstance;
	static IOCPHelper* Instance();
public:
	bool Init();
	bool ShutDown();
	bool RegHandle(IOCPHanlder* handle);
	bool _WorkThread();	
private:
	volatile bool m_run;
	volatile unsigned int m_existCount;
	unsigned int m_threadCount;
	HANDLE m_iocp;
};

