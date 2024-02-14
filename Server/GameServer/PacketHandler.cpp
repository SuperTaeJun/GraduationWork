#include "pch.h"
#include "PacketHandler.h"
array <ClientInfo, 100> clients;
//void Login_Back(const CS_LOGIN_PACKET* packet, Overlapped* overlapped);

void PacketHandler::ProcessPacket(int id, char* r_ptr)
{
	Idnum = id;
	recv_buf = r_ptr;
	char packet_type = r_ptr[1];

	switch (packet_type)
	{
	case CS_LOGIN:
		Login();
		break;
	default:
		break;
	}
}

bool PacketHandler::Login()
{
	CS_LOGIN_PACKET* packet = reinterpret_cast<CS_LOGIN_PACKET*>(recv_buf);
	cout << "recv id : " << packet->id << "," << "recv pw : " << packet->pw << endl;

	Login_Back(packet);
	return true;
}
void PacketHandler::Login_Back(const CS_LOGIN_PACKET* packet)
{
	SC_LOGIN_BACK* b_packet = new SC_LOGIN_BACK;
	b_packet->size = sizeof(SC_LOGIN_BACK);
	b_packet->type = SC_LOGIN_OK;
	strcpy_s(b_packet->id, packet->id);
	strcpy_s(b_packet->pw, packet->pw);


}

