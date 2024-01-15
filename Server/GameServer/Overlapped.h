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
	IO_CONNECT,
};

class Overlapped {
public:
	WSAOVERLAPPED	overlapped;
	IO_type			type;
	WSABUF			dataBuf;
	SOCKET			socket;
	unsigned char	messageBuffer[buffsize];
	int				recvBytes;
	int				sendBytes;
};