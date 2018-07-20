#include "stdafx.h"
#include "Interface.h"

bool CreateSocket(CreateSocketType type, UINT localIP, USHORT port, CreateSocketData &csd)
{
	SOCKET s = INVALID_SOCKET;
	if (type & CST_UDP_IOCP)
	{
		s = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	else if (type & CST_TCP_IOCP)
	{
		s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	else if (type &CST_TCP)
	{
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else if (type & CST_UDP)
	{
		s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}

	if (s == INVALID_SOCKET)
		return false;

	unsigned long arg = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&arg, sizeof(arg));
	ioctlsocket(s, FIONBIO, &arg);
	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_addr.S_un.S_addr = (localIP == 0 ? htonl(INADDR_ANY) : localIP);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	int ret = bind(s, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
	if (ret == SOCKET_ERROR)
	{
		closesocket(s);
		Log("Bind Failed:error code=%d,ip = %s,port=%d" << WSAGetLastError());
		return false;
	}
	if (localIP != 0 && addr.sin_addr.S_un.S_addr != localIP)
	{
		closesocket(s);
		Log("绑定本机指定IP失败.");
		return false;
	}
	csd.Port = ntohs(addr.sin_port);
	csd.Addr = addr;
	csd.Socket = s;
	csd.RegState = false;
	return true;
}