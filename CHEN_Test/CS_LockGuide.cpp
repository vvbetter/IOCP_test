#include "stdafx.h"
#include "CS_LockGuide.h"


CS_LockGuide::CS_LockGuide()
{
}


CS_LockGuide::~CS_LockGuide()
{
	DeleteCriticalSection(&m_cs);
}

bool CS_LockGuide::init()
{
	InitializeCriticalSection(&m_cs);
	return true;
}

void CS_LockGuide::lock()
{
	EnterCriticalSection(&m_cs);
}

void CS_LockGuide::unlock()
{
	LeaveCriticalSection(&m_cs);
}
