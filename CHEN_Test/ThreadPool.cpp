#include "stdafx.h"
#include "ThreadPool.h"
#include "Interface.h"

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
		}
		Sleep(1);
	}
}

_ThreadElem* ThreadPool::GetElem()
{
	AUTO_LOCKER(m_csLocker);
	if (m_elems.size() == 0) return NULL;
	//遍历
	
	DWORD tick = timeGetTime();
	for (auto m_elemIt = m_elems.begin(); m_elemIt != m_elems.end();)
	{
		_ThreadElem* oneElem = *m_elemIt;
		if (tick - oneElem->startTime > oneElem->interval)
		{
			oneElem->startTime = tick;
			if (oneElem->runCount == 0 && oneElem->runCount != MAX_THREAD_RUNCOUNT)
			{
				SAFE_DELETE(oneElem->pParam);
				SAFE_DELETE(oneElem);
				m_elemIt = m_elems.erase(m_elemIt);
				continue;
			}
			if (oneElem->runCount != MAX_THREAD_RUNCOUNT)
			{
				--oneElem->runCount;
			}
			return oneElem;
		}
		++m_elemIt;
	}
	return NULL;
}


ThreadPool::ThreadPool()
{
	m_bRun = false;
	m_threadNum = 0;
}


ThreadPool::~ThreadPool()
{
	timeEndPeriod(1);
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
	//初始化线程
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	DWORD m_threadCount = sysInfo.dwNumberOfProcessors + 2;
	m_bRun = true;
	for (DWORD i = 0; i < m_threadCount; ++i)
	{
		::_beginthreadex(NULL, 0, ThreadPoolWorkThread, this, 0, 0);
	}
	//初始化时间控制
	timeBeginPeriod(1);
	//初始化临界区锁
	m_csLocker.init();
	return true;
}

bool ThreadPool::AddElem(_ThreadElem* param)
{
	if (param->runCount == 0 && param->runCount != MAX_THREAD_RUNCOUNT) 
	{
		return false;
	}
	if (!HasSpace())
	{
		return false;
	}
	AUTO_LOCKER(m_csLocker);
	m_elems.push_back(param);
	return true;
}

bool ThreadPool::HasSpace()
{
	AUTO_LOCKER(m_csLocker);
	if (m_elems.size() < MAX_THREAD_ELEMS)
	{
		return true;
	}
	return false;
}
