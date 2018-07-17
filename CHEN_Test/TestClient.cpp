#include "stdafx.h"
#include "TestClient.h"


TestClient::TestClient()
{
	n = 0;
}


TestClient::~TestClient()
{
}

void TestClient::TestFunc(IThreadParam * param)
{
	TestParam* pt = (TestParam*)param;
	TestClient* p = (TestClient*)pt->p;
	Log("TestClient n=" << p->n);
	++p->n;
}
