#pragma once
#include "ThreadPool.h"

struct TestParam :public IThreadParam
{
	void* p;
	TestParam(void* np) :p(np) {}
};

class TestClient
{
public:
	TestClient();
	~TestClient();
public:
	static void TestFunc(IThreadParam* param);
private:
	volatile int n;
};

