#include "pch.h"
#include "PacketHandler.h"
//array <ClientInfo, 100> clients;
//void Login_Back(const CS_LOGIN_PACKET* packet, Overlapped* overlapped);

//void PacketHandler::ProcessPacket(int id, unsigned char* r_ptr)
//{
//	cout << "d";
//	Idnum = id;
//	recv_buf = r_ptr;
//	cl = clients[Idnum];
//	unsigned char packet_type = r_ptr[1];
//
//	switch (packet_type)
//	{
//	case CS_LOGIN:
//		Login();
//		break;
//	default:
//		break;
//	}
//}

//
//bool PacketHandler::Login()
//{
//	CS_LOGIN_PACKET* packet = reinterpret_cast<CS_LOGIN_PACKET*>(recv_buf);
//	cout << "recv id : " << packet->id << "," << "recv pw : " << packet->pw << endl;
//	Login_Back(Idnum);
//	return true;
//}
//
//int PacketHandler::get_id()
//{
//	static int g_id = 0;
//	return g_id;
//	
//}

