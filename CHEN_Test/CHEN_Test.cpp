// CHEN_Test.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "IOCPHelper.h"
#include "CLog.h"
#include <iostream>
#include <fstream>
using namespace std;

int main()
{
	IOCPHelper::Instance()->Init();
	CLog::Instance()->Init();

    return 0;
}

