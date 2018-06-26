#include "stdafx.h"
#include "IOCPHelper.h"
#include <process.h>

#define IOCP_WAIT_TIMEOUT 20

static unsigned int WINAPI WorkThread(void *Param)
{
	((IOCPHelper*)Param)->_WorkThread();
	return 0;
}

IOCPHelper* IOCPHelper::pInstance = NULL;
IOCPHelper::IOCPHelper()
{
	m_run = true;
	m_existCount = 0;
	m_threadCount = 0;
}


IOCPHelper::~IOCPHelper()
{
	if (m_run)
	{
		ShutDown();
	}
}

IOCPHelper * IOCPHelper::Instance()
{
	if (pInstance == NULL)
	{
		pInstance = new IOCPHelper();
	}
	return pInstance;
}

bool IOCPHelper::Init()
{
	Log("初始化。。。");
	int errorCode = 0;
	m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_iocp == INVALID_HANDLE_VALUE)
	{
		Log("CreateIoCompletionPort Init failed,errorcod =" << WSAGetLastError());
		return false;
	}
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	m_threadCount = sysInfo.dwNumberOfProcessors + 2;
	for (DWORD i = 0; i < m_threadCount; ++i)
	{
		::_beginthreadex(NULL, 0, WorkThread, this, 0, 0);
	}
	return true;
}

bool IOCPHelper::ShutDown()
{
	m_run = false;
	while (m_threadCount != m_existCount)
	{
		Sleep(100);
	}
	CloseHandle(m_iocp);
	Log("ShutDown!");
	return true;
}

bool IOCPHelper::RegHandle(IOCPHanlder* handle)
{
	if (CreateIoCompletionPort(handle->GetHanle(), m_iocp, (DWORD)handle, 0) == NULL)
	{
		Log("RegHandle failed, error code = " << GetLastError());
		return false;
	}
	return true;
}
//work线程可以开一个类处理
bool IOCPHelper::_WorkThread()
{
	DWORD BytesTransferred;
	LPOVERLAPPED pOverlapped;
	PerIocpData* pIocpData = NULL;
	IOCPHanlder* pHandle = NULL;
	while (m_run)
	{
		BOOL queueRet = GetQueuedCompletionStatus(m_iocp, &BytesTransferred, (PULONG_PTR)&pHandle, &pOverlapped, INFINITE);
		if (queueRet == false)
		{
			DWORD errorCode = GetLastError();
			if (ERROR_INVALID_HANDLE == errorCode)
			{
				Log("完成端口关闭，关闭线程！");
				break;
			}
			else if (WAIT_TIMEOUT == errorCode)
			{
				//TODO
			}
		}
		pIocpData = (PerIocpData*)CONTAINING_RECORD(pOverlapped, PerIocpData, overlapped);
		if (pHandle)
		{
			pHandle->Callback(pIocpData);
		}
	}
	//退出线程
	::InterlockedIncrement(&m_existCount);
	return false;
}
