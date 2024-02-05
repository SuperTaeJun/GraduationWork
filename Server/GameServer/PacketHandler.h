#pragma once
#include "CorePch.h"
#include "Overlapped.h"
#include "ClientInfo.h"
#include "lOCPServer.h"
class PacketHandler
{
public:
	PacketHandler() {};
	virtual ~PacketHandler() {};

public:
	void ProcessPacket(int id, char* r_ptr);
	bool Login_Back();

private:
	ClientInfo* cl;
	int Idnum;
	char* recv_buf;
};

