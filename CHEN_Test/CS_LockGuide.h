#pragma once
#include<Windows.h>

#define AUTO_LOCKER(x) CS_LockGuide m(x)
class CS_LockGuide
{
public:
	CS_LockGuide(CRITICAL_SECTION& cs);
	~CS_LockGuide();
private:
	CRITICAL_SECTION m_cs;
};

