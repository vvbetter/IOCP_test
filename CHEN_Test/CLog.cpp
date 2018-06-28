#include "stdafx.h"
#include "CLog.h"
#include "IOCPHelper.h"
#include <iostream>
#include <string>

CLog::CLog()
{
	m_logfile = "log.txt";
	m_bOpened = false;
	pData = NULL;
}

CLog::~CLog()
{
	CloseLog();
	SAFE_DELETE(pData);
}
CLog* CLog::pInstance = NULL;
CLog * CLog::Instance()
{
	if (pInstance == NULL)
	{
		pInstance = new CLog();
	}
	return pInstance;
}

const HANDLE CLog::GetHanle()
{
	return m_fileHandle;
}

bool CLog::Callback(PerIocpData * pData)
{
	//Log("callback");
	return true;
}

bool CLog::RegIOHandle()
{
	return IOCPHelper::Instance()->RegHandle(this);
}

bool CLog::Init()
{
	bool initSuccess = false;
	m_fileHandle = CreateFile(L"log.txt", GENERIC_WRITE,0,NULL, OPEN_EXISTING,  FILE_FLAG_OVERLAPPED,NULL);
	if (m_fileHandle == INVALID_HANDLE_VALUE)
	{
		Log("OpenFile Error. error code = " << GetLastError());
		return false;
	}
	DWORD dwCurrentFilePosition = SetFilePointer(m_fileHandle, 0, NULL, FILE_END);
	pData = new PerIocpData();
	if (!pData) return false;
	memset(pData, 0, sizeof(PerIocpData));
	pData->overlapped.Offset = dwCurrentFilePosition;
	initSuccess =  RegIOHandle();
	if (!initSuccess) return false;
	m_bOpened = true;
	return true;
}

bool CLog::CloseLog()
{
	if(m_bOpened)
	{
		CloseHandle(m_fileHandle);
	}
	return true;
}

bool CLog::TranceLog(const char* str, ...)
{
	if (m_bOpened == false)
	{
		return false;
	}
	va_list var;
	va_start(var, str);
	int nCount = _vscprintf(str, var);
	char *pBuffer = new char[nCount + 1];
	memset(pBuffer, 0, nCount + 1);
	if (pBuffer)
	{
		vsprintf_s(pBuffer, nCount + 1, str, var);
	}
#ifdef _DEBUG
	cout << pBuffer << endl;
#endif // _DEBUG

	INT writenLen = 0;
	DWORD temp;
	while (writenLen < nCount)
	{
		DWORD lastWriteLen = (nCount - writenLen) >= IOCP_BUFFER_SIZE ? IOCP_BUFFER_SIZE : (nCount - writenLen);
		memset(pData->buffer, 0, IOCP_BUFFER_SIZE);
		memcpy_s(pData->buffer , IOCP_BUFFER_SIZE, pBuffer + writenLen, lastWriteLen);
		BOOL ret = WriteFile(m_fileHandle, pData->buffer, lastWriteLen, &temp, &pData->overlapped);
		pData->overlapped.Offset += lastWriteLen;
		if (!ret)
		{
			DWORD retCode = GetLastError();
			if (retCode != ERROR_IO_PENDING)
			{
				Log("write file error: " << retCode);
			}
		}
		writenLen += lastWriteLen;
	}
	SAFE_DELETE(pBuffer);
	return true;
}
