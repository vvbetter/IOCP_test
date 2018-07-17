#pragma once
#include "Interface.h"
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
	INT runCount;	//
	DWORD startTime;//ms timeGetTime()
	DWORD interval; //ms
	_ThreadElem( void(*cb)(IThreadParam*) = NULL , IThreadParam* param = NULL, INT rc = 0, DWORD st = 0, DWORD it = 0)
		:callback(cb), pParam(param), runCount(rc), startTime(st), interval(it) {}
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
	bool Init();
	bool AddElem(_ThreadElem*);
	bool HasSpace();
	void ThreadFunc();
private:
	_ThreadElem* GetElem();
private:
	bool m_bRun;
	volatile unsigned int m_threadNum;
	CRITICAL_SECTION m_cs;
	list<_ThreadElem*> m_elems;
	list<_ThreadElem*>::iterator m_elemIt;
};

