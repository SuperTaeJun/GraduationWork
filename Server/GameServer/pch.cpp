#include "pch.h"


array <ClientInfo, 100> clients;

void Login_Back(int _s_id)
{
	SC_LOGIN_BACK* b_packet = new SC_LOGIN_BACK;
	b_packet->size = sizeof(SC_LOGIN_BACK);
	b_packet->type = SC_LOGIN_OK;
	b_packet->cl_id = _s_id;
	/*strcpy_s(b_packet->id, packet->id);
	strcpy_s(b_packet->pw, packet->pw);*/
	clients[_s_id].c_send(sizeof(b_packet), &b_packet);

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
		cout << _s_id << endl;
		printf_s("[INFO] 로그인 시도 {%s}/{%s}\n", packet->id, packet->pw);
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