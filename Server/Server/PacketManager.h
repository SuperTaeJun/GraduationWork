#pragma once
#include "pch.h"

class IOCPServer;
class Room;
class RoomManager;

class PacketManager
{
public:
	PacketManager() {};
	virtual ~PacketManager() {};
	void ProcessPacket(int s_id, unsigned char* p);
	int get_id();
private:
	CLIENT* cl = nullptr;
	int IdNum = 0;
	unsigned char* RecvBuf = nullptr;;
};

