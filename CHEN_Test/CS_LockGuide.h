#pragma once
#include<Windows.h>
#include <mutex>

class CS_LockGuide
{
public:
	CS_LockGuide();
	~CS_LockGuide();
	bool init();
	void lock();
	void unlock();
private:
	CRITICAL_SECTION m_cs;
};
#define AUTO_LOCKER(x) std::lock_guard<CS_LockGuide> lock(x)
