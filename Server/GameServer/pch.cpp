#include "pch.h"


array <ClientInfo, 100> clients;

void Login_Back(int _s_id)
{
	SC_LOGIN_BACK b_packet;
	b_packet.size = sizeof(SC_LOGIN_BACK);
	b_packet.type = SC_LOGIN_OK;
	clients[_s_id].c_send(sizeof(b_packet), &b_packet);
	cout << "b_packet - size : " << sizeof(SC_LOGIN_BACK) << endl;
}
void process_packet(int _s_id, unsigned char* p)
{
	unsigned char packet_type = p[1];
	ClientInfo& cl = clients[_s_id];
	switch (packet_type)
	{
	case CS_LOGIN: {
		printf("login\n");
		CS_LOGIN_PACKET* packet = reinterpret_cast<CS_LOGIN_PACKET*>(p);
		ClientInfo& cl = clients[_s_id];
		cout << "client 접속 id : " << _s_id << endl;
		cout << "로그인 시도  :" << packet->id << packet->pw << endl;
		cout << packet->id << " 로그인 성공" << endl;
		Login_Back(_s_id);
		break;
	}
	default:
		break;
	}
}
int get_id()
{
	static int g_id = 0;

	for (int i = 0; i < 100; ++i) {
		if (ST_FREE == clients[i].cl_state) {
			clients[i].cl_state = ST_ACCEPT;
			return i;
		}
	}
	cout << "Maximum Number of Clients Overflow!!\n";
	return -1;
}