#include "pch.h"


array <ClientInfo*, 100> clients;

void Login_Back(int _s_id)
{
	SC_LOGIN_BACK* b_packet = new SC_LOGIN_BACK;
	b_packet->size = sizeof(SC_LOGIN_BACK);
	b_packet->type = SC_LOGIN_OK;
	b_packet->cl_id = _s_id;
	/*strcpy_s(b_packet->id, packet->id);
	strcpy_s(b_packet->pw, packet->pw);*/
	clients[_s_id]->c_send(sizeof(b_packet), &b_packet);

}
