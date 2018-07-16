#include "stdafx.h"
#include "ThreadPool.h"
#include "Interface.h"
#include "CS_LockGuide.h"

static unsigned int WINAPI ThreadPoolWorkThread(void *Param)
{
	((ThreadPool*)Param)->ThreadFunc();
	return 0;
}

void ThreadPool::ThreadFunc()
{
	UINT threadNumber = ::InterlockedIncrement(&m_threadNum);
	Log("ThreadPool the " << threadNumber << "th thread start!");
	while (m_bRun)
	{
		_ThreadElem* oneElem = GetElem();
		if (NULL != oneElem)
		{
			oneElem->callback(oneElem->pParam);
			delete oneElem->pParam;
			delete oneElem;
		}
		Sleep(1);
	}
}

_ThreadElem* ThreadPool::GetElem()
{
	AUTO_LOCKER(m_cs);
	if (m_elems.size() == 0) return NULL;
	if (m_elemIt == m_elems.end())
	{
		m_elemIt = m_elems.begin();
	}
	_ThreadElem* p = *m_elemIt;
	if (p->runCount == MAX_THREAD_RUNCOUNT)
	{
		++m_elemIt;
	}
	else if (p->runCount <= 0)
	{
		m_elemIt = m_elems.erase(m_elemIt);
		return NULL;
	}
	else
	{
		--p->runCount;
		++m_elemIt;
	}
	return p;
}


ThreadPool::ThreadPool()
{
	m_bRun = false;
	m_threadNum = 0;
}


ThreadPool::~ThreadPool()
{
}
ThreadPool* ThreadPool::instance = NULL;
ThreadPool* ThreadPool::Instance()
{
	if (instance == NULL)
	{
		instance = new ThreadPool();
	}
	return instance;
}

bool ThreadPool::Init()
{
	Log("初始化线程池。。。");
	int errorCode = 0;
	//初始化临界区
	InitializeCriticalSection(&m_cs);
	//初始化线程
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	DWORD m_threadCount = sysInfo.dwNumberOfProcessors + 2;
	m_bRun = true;
	for (DWORD i = 0; i < m_threadCount; ++i)
	{
		::_beginthreadex(NULL, 0, ThreadPoolWorkThread, this, 0, 0);
	}
	//初始化变量
	m_elemIt = m_elems.begin();
	return true;
}

bool ThreadPool::AddElem(_ThreadElem* param)
{
	if (param->runCount <= 0 && param->runCount != MAX_THREAD_RUNCOUNT) 
	{
		return false;
	}
	if (!HasSpace())
	{
		return false;
	}
	AUTO_LOCKER(m_cs);
	m_elems.push_back(param);
	return true;
}

bool ThreadPool::HasSpace()
{
	AUTO_LOCKER(m_cs);
	if (m_elems.size() < MAX_THREAD_ELEMS)
	{
		return true;
	}
	return false;
}
