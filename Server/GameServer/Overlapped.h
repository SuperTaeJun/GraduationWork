#pragma once
#include <process.h>
#include "pch.h"
#include "CorePch.h"

const int buffsize = 1000;

enum IO_type
{
	IO_RECV,
	IO_SEND,
	IO_ACCEPT,
	//IO_CONNECT,
};

class Overlapped {
public:

	WSAOVERLAPPED	overlapped;
	WSABUF			wsabuf;
	//SOCKET			socket;
	unsigned char	recvBuffer[buffsize + 1];
	/*int				recvBytes;
	int				sendBytes;*/
	IO_type			type; // read, write, accept, connect ...
public:
	Overlapped(IO_type type, char bytes, void* mess) : type(type)
	{
		ZeroMemory(&overlapped, sizeof(overlapped));
		wsabuf.buf = reinterpret_cast<char*>(recvBuffer);
		wsabuf.len = bytes;
		memcpy(recvBuffer, mess, bytes);
	}
	Overlapped(IO_type type) : type(type)
	{
		ZeroMemory(&overlapped, sizeof(overlapped));
		wsabuf.buf = {};
		wsabuf.len = {};
	}
	Overlapped()
	{
		type = IO_RECV;
		ZeroMemory(&overlapped, sizeof(overlapped));
		wsabuf.buf = {};
		wsabuf.len = {};
	}
	~Overlapped()
	{

	}
};

