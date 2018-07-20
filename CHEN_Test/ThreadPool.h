#pragma once
#include "Interface.h"
#include "CS_LockGuide.h"
#include <process.h>
#include <list>
#include <Windows.h>
#pragma comment(lib, "winmm.lib")

#define MAX_THREAD_ELEMS 2000
#define MAX_THREAD_RUNCOUNT -1
using namespace std;

__interface IThreadParam
{

};

struct _ThreadElem
{
	void(*callback)(IThreadParam*);
	IThreadParam* pParam;
	DWORD runCount;	//
	DWORD startTime;//ms timeGetTime()
	DWORD interval; //ms
	_ThreadElem( void(*cb)(IThreadParam*) = NULL , IThreadParam* param = NULL, DWORD runCount = 0, DWORD startTime = 0, DWORD interval = 0)
		:callback(cb), pParam(param), runCount(runCount), startTime(startTime), interval(interval) {}
};

class ThreadPool
{
private:
	ThreadPool();
	~ThreadPool();
public:
	static ThreadPool* instance;
	static ThreadPool* Instance();
public:
	bool Init(UINT threadNum);
	bool AddElem(_ThreadElem*);
	bool HasSpace();
	void ThreadFunc();
private:
	_ThreadElem* GetElem();
private:
	bool m_bRun;
	volatile unsigned int m_threadNum;
	CS_LockGuide m_csLocker;
	list<_ThreadElem*> m_elems;
};

