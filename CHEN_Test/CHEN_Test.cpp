// CHEN_Test.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "IOCPHelper.h"
#include "CLog.h"
#include <iostream>
#include <fstream>
using namespace std;

int sum(int a, int b)
{
	return a + b;
}

int main()
{
	IOCPHelper::Instance()->Init();
	CLog::Instance()->Init();
	volatile int n = 10;
	while (n--)
	{
		TRANSLOG("%d 14:41:2 2.000000 9.000000 0.003950 -0.220000, %d%d\n", n, n + 2, n - 2);
	}


    return 0;
}

