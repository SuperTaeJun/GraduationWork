#include "pch.h"
#include "PacketHandler.h"
void PacketHandler::ProcessPacket(int id, char* r_ptr)
{
	Idnum = id;
	recv_buf = r_ptr;
	char packet_type = r_ptr[1];

	switch (packet_type)
	{
	case CS_LOGIN:
		Login_Back();
		break;
	default:
		break;
	}
}

bool PacketHandler::Login_Back()
{
	return true;
}
