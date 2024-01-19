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
	SOCKET			socket;
	char			recvBuffer[buffsize + 1];
	int				recvBytes;
	int				sendBytes;
	IO_type			type; // read, write, accept, connect ...
};

