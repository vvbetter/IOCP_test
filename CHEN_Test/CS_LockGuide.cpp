#include "stdafx.h"
#include "CS_LockGuide.h"


CS_LockGuide::CS_LockGuide(CRITICAL_SECTION& cs) : m_cs(cs)
{
	EnterCriticalSection(&m_cs);
}


CS_LockGuide::~CS_LockGuide()
{
	LeaveCriticalSection(&m_cs);
}
