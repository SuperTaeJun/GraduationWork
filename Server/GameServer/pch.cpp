#include "pch.h"


array <ClientInfo, 100> clients;

void Login_Back(int _s_id)
{
	SC_LOGIN_BACK b_packet;
	b_packet.size = sizeof(b_packet);
	b_packet.type = SC_LOGIN_OK;
	b_packet.cl_id = _s_id;
	if(clients[_s_id].c_send(sizeof(b_packet), &b_packet))
		cout << "b_packet - size : " << sizeof(SC_LOGIN_BACK) << endl;
}
void Send_Player(int _s_id, int enm)
{
	CS_MOVE_PACKET packet;
	//packet.id = enm;
	//packet.size = sizeof(packet);
	//packet.type = SC_OWN_MOVE;
	//packet.x = clients[enm].x;
	//packet.y = clients[enm].y;
	packet.id = enm;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE_PLAYER;
	packet.x = clients[enm].x;
	packet.y = clients[enm].y;
	packet.z = clients[enm].z;
	packet.yaw = clients[enm].Yaw;
	packet.vx = clients[enm].VX;
	packet.vy = clients[enm].VY;
	packet.vz = clients[enm].VZ;
	clients[_s_id].c_send(sizeof(packet), &packet);
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

		for (auto& other : clients) {
			if (other.cl_id == _s_id) {
				continue;
			}
			SC_PLAYER_SYNC _packet;
			_packet.id = _s_id;
			_packet.object_type = 0;
			_packet.size = sizeof(packet);
			_packet.type = SC_OTHER_PLAYER;
			_packet.x = cl.x;
			_packet.y = cl.y;
			_packet.z = cl.z;
			other.c_send(sizeof(_packet), &_packet);
		}
		// 새로 접속한 플레이어에게 주위 객체 정보를 보낸다
		for (auto& other : clients) {
			if (other.cl_id == _s_id) continue;
			SC_PLAYER_SYNC packet;
			packet.id = other.cl_id;
			packet.object_type = 0;
			packet.size = sizeof(packet);
			packet.type = SC_OTHER_PLAYER;
			packet.x = other.x;
			packet.y = other.y;
			packet.z = other.z;
			packet.yaw = other.Yaw;

			cout << "[Send put object] id : location : " << packet.id <<"," << packet.x << "," << packet.y << endl;
			cl.c_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_MOVE:
	{
		printf("Move\n");
		CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(p);
		ClientInfo& cl = clients[_s_id];
		cl.x = packet->x;
		cl.y = packet->y;
		cl.z = packet->z;
		cl.Yaw = packet->yaw;
		cl.VX = packet->vx;
		cl.VY = packet->vy;
		cl.VZ = packet->vz;

		cout << "x: " << packet->x << " y: " << packet->y << " z : " << packet->z << endl;
		//클라 recv 확인용
		for (auto& other : clients) {
			if (other.cl_id == _s_id)
				continue;
			Send_Player(other.cl_id, cl.cl_id);
			//	cout <<"움직인 플레이어" << cl._s_id << "보낼 플레이어" << other._s_id << endl;

		}
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