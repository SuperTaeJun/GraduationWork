#pragma once
#include "CorePch.h"
#include "Overlapped.h"
#include "protocol.h"
#include "ClientInfo.h"
#include "lOCPServer.h"

//extern array <ClientInfo, 100> clients;
class PacketHandler
{
public:
	PacketHandler() {};
	virtual ~PacketHandler() {};

public:
	void ProcessPacket(int id, char* r_ptr);
	bool Login();
private:
	ClientInfo* cl;
	int Idnum;
	char* recv_buf;
};

