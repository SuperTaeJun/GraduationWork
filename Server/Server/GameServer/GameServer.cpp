﻿//----------------------------------------------------------------------------------------------------------------------------------------------
// GameServer.cpp 파일
//----------------------------------------------------------------------------------------------------------------------------------------------

#include "pch.h"
#include "CorePch.h"
#include "CLIENT.h"
#include "Overlap.h"
#include <thread>
#include <string>
#include <mutex>
#include <atomic>
using namespace std;
mutex m;
HANDLE g_h_iocp;
SOCKET sever_socket;
concurrency::concurrent_priority_queue <timer_ev> timer_q;
array <CLIENT, MAX_USER> clients;
atomic<int> ready_count = 0;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

void show_err();
int get_id();
void send_select_character_type_packet(int _s_id);
void send_login_ok_packet(int _s_id);
//void send_login_fail_packet(int _s_id);
void send_move_packet(int _id, int target);
void send_change_hp(int _s_id);
//void send_remove_object(int _s_id, int victim);
void send_put_object(int _s_id, int target);
void Disconnect(int _s_id);
void send_ready_packet(int _s_id);
//void send_damage_packet(int _s_id);
void worker_thread();

int main()
{
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
	::WSAStartup(MAKEWORD(2, 2), &WSAData);
	sever_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sever_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(sever_socket, SOMAXCONN);
	cout << "서버 시작" << endl;
	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(sever_socket), g_h_iocp, 0, 0);

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char   accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	Overlap   accept_ex;
	*(reinterpret_cast<SOCKET*>(&accept_ex._net_buf)) = c_socket;
	ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
	accept_ex._op = IO_ACCEPT;

	AcceptEx(sever_socket, c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);

	for (int i = 0; i < MAX_USER; ++i)
		clients[i]._s_id = i;

	vector <thread> worker_threads;



	// 시스템 정보 가져옴
	//SYSTEM_INFO sysInfo;
	//GetSystemInfo(&sysInfo);
	//printf_s("[INFO] CPU 쓰레드 갯수 : %d\n", sysInfo.dwNumberOfProcessors);
	// 적절한 작업 스레드의 갯수는 (CPU * 2) + 1
	//int nThreadCnt = sysInfo.dwNumberOfProcessors;
	//int nThreadCnt = 5;


	for (int i = 0; i < 16; ++i)
		worker_threads.emplace_back(worker_thread);

	for (auto& th : worker_threads)
		th.join();

	for (auto& cl : clients) {
		if (ST_INGAME == cl._state)
			Disconnect(cl._s_id);
	}
	closesocket(sever_socket);
	WSACleanup();
}

void show_err() {
	cout << "error" << endl;
}



//새로운 id(인덱스) 할당
int get_id()
{
	static int g_id = 0;

	for (int i = 0; i < MAX_USER; ++i) {
		clients[i].state_lock.lock();
		if (ST_FREE == clients[i]._state) {
			clients[i]._state = ST_ACCEPT;
			clients[i].state_lock.unlock();
			return i;
		}
		else clients[i].state_lock.unlock();
	}
	cout << "Maximum Number of Clients Overflow!!\n";
	return -1;
}

//로그인 허용
void send_login_ok_packet(int _s_id)
{
	SC_LOGIN_BACK packet;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_OK;
	packet.id = _s_id;
	cout << "_s_id" << _s_id << endl;
	//packet.clientid = _s_id;
	/*packet.x = clients[_s_id].x;
	packet.y = clients[_s_id].y;
	packet.z = clients[_s_id].z;
	packet.yaw = clients[_s_id].Yaw;*/
	//packet.p_type = clients[_s_id].p_type;
	/*packet.Yaw = clients[_s_id].Yaw;
	packet.Pitch = clients[_s_id].Pitch;
	packet.Roll = clients[_s_id].Roll;*/
	//printf("[Send login ok] id : %d(%)\n", packet.clientid);
	clients[_s_id].do_send(sizeof(packet), &packet);
}
void send_select_character_type_packet(int _s_id)
{
	SC_SELECT_CHARACTER_BACK packet;
	packet.size = sizeof(packet);
	packet.type = SC_CHAR_BACK;
	packet.clientid = _s_id;
	packet.x = clients[_s_id].x;
	packet.y = clients[_s_id].y;
	packet.z = clients[_s_id].z;
	packet.p_type = clients[_s_id].p_type;
	clients[_s_id].do_send(sizeof(packet), &packet);
}


//로그인 실패


//오브젝트 생성
void send_put_object(int _s_id, int target)
{
	SC_PLAYER_SYNC packet;
	packet.id = target;
	packet.size = sizeof(packet);
	packet.type = SC_OTHER_PLAYER;
	packet.x = clients[target].x;
	packet.y = clients[target].y;
	packet.z = clients[target].z;

	strcpy_s(packet.name, clients[target].name);
	//packet.object_type = 0;
	clients[_s_id].do_send(sizeof(packet), &packet);
}


//해제
void Disconnect(int _s_id)
{
	CLIENT& cl = clients[_s_id];
	/*cl.vl.lock();
	unordered_set <int> my_vl = cl.viewlist;
	cl.vl.unlock();

	for (auto& other : my_vl) {
		CLIENT& target = clients[other];

		if (ST_INGAME != target._state)
			continue;
		target.vl.lock();
		if (0 != target.viewlist.count(_s_id)) {
			target.viewlist.erase(_s_id);
			target.vl.unlock();
			send_remove_object(other, _s_id);
		}
		else target.vl.unlock();
	}*/
	clients[_s_id].state_lock.lock();
	clients[_s_id]._state = ST_FREE;
	clients[_s_id].state_lock.unlock();
	closesocket(clients[_s_id]._socket);
	cout << "------------연결 종료------------" << endl;
}


//패킷 판별
void process_packet(int s_id, char* p)
{
	unsigned char packet_type = p[1];
	CLIENT& cl = clients[s_id];
	//cout << "packet type :" << to_string(packet_type) << endl;
	switch (packet_type) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* packet = reinterpret_cast<CS_LOGIN_PACKET*>(p);

		CLIENT& cl = clients[s_id];
		cout << "[Recv login] ID :" << packet->id << ", PASSWORD : " << packet->pw << endl;
		cl.state_lock.lock();
		cl._state = ST_INGAME;
		cl.state_lock.unlock();
		/*cl.x = packet->x;
		cl.y = packet->y;
		cl.z = packet->z;*/
		//cl.p_type = packet->p_type;
		//cl.Yaw = s_id * 40.0f;
		//cout << "cl.x : " << cl.x << endl;
		send_login_ok_packet(cl._s_id);
		cout << "플레이어[" << s_id << "]" << " 로그인 성공" << endl;

		//새로 접속한 플레이어의 정보를 주위 플레이어에게 보낸다

		break;

	}
	case CS_SELECT_CHAR: {

		CS_SELECT_CHARACTER* packet = reinterpret_cast<CS_SELECT_CHARACTER*>(p);
		CLIENT& cl = clients[s_id];
		cl.x = packet->x;
		cl.y = packet->y;
		cl.z = packet->z;
		cl.p_type = packet->p_type;
		send_select_character_type_packet(cl._s_id);


		cout << "cl._s_id : " << cl._s_id << ",  " << cl.p_type << endl;
		//m.lock();
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();

			SC_PLAYER_SYNC packet;
			packet.id = cl._s_id;
			strcpy_s(packet.name, cl.name);
			//packet.object_type = 0;
			packet.size = sizeof(packet);
			packet.type = SC_OTHER_PLAYER;
			packet.x = cl.x;
			packet.y = cl.y;
			packet.z = cl.z;
			packet.yaw = cl.Yaw;
			packet.Max_speed = cl.Max_Speed;
			packet.p_type = cl.p_type;
			printf_s("[Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);
		}
		//m.unlock();
		//m.lock();
		// 새로 접속한 플레이어에게 주위 객체 정보를 보낸다
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();


			SC_PLAYER_SYNC packet;
			packet.id = other._s_id;
			strcpy_s(packet.name, other.name);
			//packet.object_type = 0;
			packet.size = sizeof(packet);
			packet.type = SC_OTHER_PLAYER;
			packet.x = other.x;
			packet.y = other.y;
			packet.z = other.z;
			packet.yaw = other.Yaw;
			packet.Max_speed = other.Max_Speed;
			packet.p_type = other.p_type;
			printf_s("[어떤 클라의 Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);

			cl.do_send(sizeof(packet), &packet);

		}
		//m.unlock();

		break;
	}
	case CS_MOVE_Packet: {
		//cout << "들어옴?" << endl;
		CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cl.x = packet->x;
		cl.y = packet->y;
		cl.z = packet->z;
		cl.Yaw = packet->yaw;
		cl.VX = packet->vx;
		cl.VY = packet->vy;
		cl.VZ = packet->vz;
		cl.Max_Speed = packet->Max_speed;
		//cout << "플레이어[" << packet->id << "]" << "  x:" << packet->x << endl;
		//cout <<"플레이어["<< packet->id<<"]" << "  x:" << packet->vx << " y:" << packet->y << " z:" << packet->z << "speed : " << packet->speed << endl;
		//클라 recv 확인용

		for (auto& other : clients) {
			if (other._s_id == s_id)
				continue;
			if (ST_INGAME != other._state)
				continue;
			send_move_packet(other._s_id, cl._s_id);
			//cout << "움직인 플레이어" << cl._s_id << "보낼 플레이어" << other._s_id << endl;

		}
		break;
	}
	case CS_SELECT_WEP:
	{
		CS_SELECT_WEAPO* packet = reinterpret_cast<CS_SELECT_WEAPO*>(p);
		CLIENT& cl = clients[packet->id];
		cl.w_type = packet->weapon_type;
		cout << "무기 타입" << cl.w_type << endl;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			SC_SYNC_WEAPO packet;
			packet.id = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_OTHER_WEAPO;
			packet.weapon_type = cl.w_type;
			//printf_s("[Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);
		}
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			SC_SYNC_WEAPO packet;
			packet.id = other._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_OTHER_WEAPO;
			packet.weapon_type = other.w_type;
			//printf_s("[어떤 클라의 Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			cl.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_READY:
	{
		CS_READY_PACKET* packet = reinterpret_cast<CS_READY_PACKET*>(p);
		CLIENT& cl = clients[s_id];
		ready_count++;
		cout << "ready_count" << ready_count << endl;
		if (ready_count >= 2)
		{
			for (auto& player : clients) {
				if (ST_INGAME != player._state)
					continue;
				m.lock();
				send_ready_packet(player._s_id);
				cout << "보낼 플레이어" << player._s_id << endl;
				m.unlock();
			}
			//cl._state = ST_INGAME;
		}
		break;
	}
	case CS_ATTACK: {
		CS_ATTACK_PLAYER* packet = reinterpret_cast<CS_ATTACK_PLAYER*>(p);
		CLIENT& cl = clients[packet->attack_id];
		cl.s_x = packet->sx;
		cl.s_y = packet->sy;
		cl.s_z = packet->sz;
		cl.e_x = packet->ex;
		cl.e_y = packet->ey;
		cl.e_z = packet->ez;
		cout << "cl.s_x" << cl.s_x << "cl.e_x" << cl.e_x << endl;
		/*	send_damage_packet(packet->attack_id);*/
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			SC_ATTACK_PLAYER packet;
			packet.clientid = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_ATTACK;
			packet.sx = cl.s_x;
			packet.sy = cl.s_y;
			packet.sz = cl.s_z;
			packet.ex = cl.e_x;
			packet.ey = cl.e_y;
			packet.ez = cl.e_z;
			//	//packet.weapon_type = cl.w_type;
			////printf_s("[Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			//	cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_SHOTGUN_BEAM: {
		CS_SHOTGUN_BEAM_PACKET* packet = reinterpret_cast<CS_SHOTGUN_BEAM_PACKET*>(p);
		CLIENT& cl = clients[packet->attackid];
		cl.s_x = packet->sx;
		cl.s_y = packet->sy;
		cl.s_z = packet->sz;
		//--------------------
		cl.ex0 = packet->ex0;
		cl.ey0 = packet->ey0;
		cl.ez0 = packet->ez0;
		cout << cl.ex0 << "a" << cl.ey0 << cl.ez0 << endl;
		cl.ex1 = packet->ex1;
		cl.ey1 = packet->ey1;
		cl.ez1 = packet->ez1;
		cout << cl.ex1 << "a" << cl.ey1 << cl.ez1 << endl;
		cl.ex2 = packet->ex2;
		cl.ey2 = packet->ey2;
		cl.ez2 = packet->ez2;
		cl.ex3 = packet->ex3;
		cl.ey3 = packet->ey3;
		cl.ez3 = packet->ez3;
		cl.ex4 = packet->ex4;
		cl.ey4 = packet->ey4;
		cl.ez4 = packet->ez4;
		cl.ex5 = packet->ex5;
		cl.ey5 = packet->ey5;
		cl.ez5 = packet->ez5;
		cout << cl.ex5 << "a" << cl.ey5 << cl.ez5 << endl;
		cl.ex6 = packet->ex6;
		cl.ey6 = packet->ey6;
		cl.ez6 = packet->ez6;
		cl.ex7 = packet->ex7;
		cl.ey7 = packet->ey7;
		cl.ez7 = packet->ez7;
		cl.ex8 = packet->ex8;
		cl.ey8 = packet->ey8;
		cl.ez8 = packet->ez8;
		/*cl.ex9 = packet->ex9;
		cl.ey9 = packet->ey9;
		cl.ez9 = packet->ey9;*/
		//--------------------
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_SHOTGUN_BEAM_PACKET packet;
			packet.attackid = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_SHOTGUN_BEAM;
			packet.sx = cl.s_x;
			packet.sy = cl.s_y;
			packet.sz = cl.s_z;
			packet.ex0 = cl.ex0;
			packet.ey0 = cl.ey0;
			packet.ey0 = cl.ez0;
			packet.ex1 = cl.ex1;
			packet.ey1 = cl.ey1;
			packet.ez1 = cl.ez1;
			packet.ex2 = cl.ex2;
			packet.ey2 = cl.ey2;
			packet.ez2 = cl.ez2;
			packet.ex3 = cl.ex3;
			packet.ey3 = cl.ey3;
			packet.ez3 = cl.ez3;
			packet.ex4 = cl.ex4;
			packet.ey4 = cl.ey4;
			packet.ez4 = cl.ez4;
			packet.ex5 = cl.ex5;
			packet.ey5 = cl.ey5;
			packet.ez5 = cl.ez5;
			packet.ex6 = cl.ex6;
			packet.ey6 = cl.ey6;
			packet.ez6 = cl.ez6;
			packet.ex7 = cl.ex7;
			packet.ey7 = cl.ey7;
			packet.ez7 = cl.ez7;
			packet.ex8 = cl.ex8;
			packet.ey8 = cl.ey8;
			packet.ez8 = cl.ez8;
			/*packet.ex9 = cl.ex9;
			packet.ey9 = cl.ey9;
			packet.ez9 = cl.ez9;*/

			cout << cl.ex0 << "a" << cl.ey0 << cl.ez0;
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);
		
		}
		break;
	}
	case CS_SHOTGUN_DAMAGED: {
		CS_SHOTGUN_DAMAGED_PACKET* packet = reinterpret_cast<CS_SHOTGUN_DAMAGED_PACKET*>(p);
		clients[packet->damaged_id]._hp -= packet->damage;
		send_change_hp(packet->damaged_id);
		clients[packet->damaged_id1]._hp -= packet->damage1;
		send_change_hp(packet->damaged_id1);
		clients[packet->damaged_id2]._hp -= packet->damage2;
		send_change_hp(packet->damaged_id2);

		//SC_SHOTGUN_DAMAGED_CHANGE_PACKET repacket;
		//repacket.size = sizeof(repacket);
		//repacket.type = SC_SHOTGUN_DAMAGED;
		//repacket.damaged_id1 = clients[packet->damaged_id]._s_id;
		//repacket.damaged_id2 = clients[packet->damaged_id1]._s_id;
		//repacket.damaged_id3 = clients[packet->damaged_id2]._s_id;
		//repacket.newhp1 = clients[packet->damaged_id]._hp;
		//repacket.newhp2 = clients[packet->damaged_id1]._hp;
		//repacket.newhp3 = clients[packet->damaged_id2]._hp;
		//clients[packet->damaged_id].do_send(sizeof(repacket), &repacket);
		//clients[packet->damaged_id1].do_send(sizeof(repacket), &repacket);
		//clients[packet->damaged_id2].do_send(sizeof(repacket), &repacket);
		break;
	}
	case CS_HIT_EFFECT: {
		CS_EFFECT_PACKET* packet = reinterpret_cast<CS_EFFECT_PACKET*>(p);
		CLIENT& cl = clients[packet->attack_id];
		cl.s_x = packet->lx;
		cl.s_y = packet->ly;
		cl.s_z = packet->lz;
		cl.Pitch = packet->r_pitch;
		cl.Yaw = packet->r_yaw;
		cl.Roll = packet->r_roll;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_EFFECT_PACKET packet;
			packet.attack_id = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_EFFECT;
			packet.lx = cl.s_x;
			packet.ly = cl.s_y;
			packet.lz = cl.s_z;
			packet.r_pitch = cl.Pitch;
			packet.r_yaw = cl.Yaw;
			packet.r_roll = cl.Roll;
			//packet.weapon_type = cl.w_type;
		//printf_s("[Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);

		}
		break;

	}
	case CS_DAMAGE: {
		CS_DAMAGE_PACKET* packet = reinterpret_cast<CS_DAMAGE_PACKET*>(p);
		CLIENT& cl = clients[packet->damaged_id];
		//데미지 저장
		cl._hp -= packet->damage;
		send_change_hp(cl._s_id);
		break;
	}
	default:
		cout << " 오류패킷타입 : " << p << endl;
		break;
	}
}
	

//워크 쓰레드
void worker_thread()
{
	while (1) {
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int _s_id = static_cast<int>(iocp_key);
		Overlap* exp_over = reinterpret_cast<Overlap*>(p_over);
		if (FALSE == ret) {
			int err_no = WSAGetLastError();
			cout << "GQCS Error : ";
			error_display(err_no);
			cout << endl;
			Disconnect(_s_id);
			if (exp_over->_op == IO_SEND)
				delete exp_over;
			continue;
		}

		switch (exp_over->_op) {
		case IO_RECV: {
			if (num_byte == 0) {
				cout << "연결종료" << endl;
				Disconnect(_s_id);
				continue;
			}
			CLIENT& cl = clients[_s_id];
			int remain_data = num_byte + cl._prev_size;
			char* packet_start = exp_over->_net_buf;
			int packet_size = packet_start[0];

			while (packet_size <= remain_data) {
				process_packet(_s_id, packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0) packet_size = packet_start[0];
				else break;
			}

			if (0 < remain_data) {
				cl._prev_size = remain_data;
				memcpy(&exp_over->_net_buf, packet_start, remain_data);
			}
			cl.do_recv();
			break;
		}
		case IO_SEND: {
			if (num_byte != exp_over->_wsa_buf.len) {
				cout << "send 에러" << endl;
				Disconnect(_s_id);
			}
			delete exp_over;
			break;
		}
		case IO_ACCEPT: {
			cout << "Accept Completed.\n";
			SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->_net_buf));

			int n__s_id = get_id();
			if (-1 == n__s_id) {
				cout << "user over.\n";
			}
			else {
				CLIENT& cl = clients[n__s_id];
				/*	cl.x = rand() % ReZone_WIDTH;
					cl.y = rand() % ReZone_HEIGHT;
					cl.z = 0;
					cl.Yaw = 0;
					cl.Pitch = 0;
					cl.Roll = 0;*/
					//cl.x = 200;
					//cl.y = 200;
				cl.state_lock.lock();
				cl._s_id = n__s_id;
				cl._state = ST_ACCEPT;
				cl.state_lock.unlock();
				cl._prev_size = 0;
				cl._recv_over._op = IO_RECV;
				cl._recv_over._wsa_buf.buf = reinterpret_cast<char*>(cl._recv_over._net_buf);
				cl._recv_over._wsa_buf.len = sizeof(cl._recv_over._net_buf);
				ZeroMemory(&cl._recv_over._wsa_over, sizeof(cl._recv_over._wsa_over));
				cl._socket = c_socket;

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, n__s_id, 0);
				cl.do_recv();
			}

			ZeroMemory(&exp_over->_wsa_over, sizeof(exp_over->_wsa_over));
			c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			*(reinterpret_cast<SOCKET*>(exp_over->_net_buf)) = c_socket;
			AcceptEx(sever_socket, c_socket, exp_over->_net_buf + 8, 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, NULL, &exp_over->_wsa_over);

		}
					  break;
		}
	}
}

//이동
void send_move_packet(int _id, int target)
{
	CS_MOVE_PACKET packet;
	packet.id = target;
	packet.size = sizeof(packet);
	packet.type = SC_MOVE_PLAYER;
	packet.x = clients[target].x;
	packet.y = clients[target].y;
	packet.z = clients[target].z;
	packet.yaw = clients[target].Yaw;
	packet.vx = clients[target].VX;
	packet.vy = clients[target].VY;
	packet.vz = clients[target].VZ;
	packet.Max_speed = clients[target].Max_Speed;
	//packet.move_time = clients[target].last_move_time;

	//printf_s("[Send move] id : %d, location : (%f,%f,%f), yaw : %f,  v : (%f,%f,%f)\n", packet.sessionID, packet.x, packet.y, packet.z, packet.yaw, packet.vx, packet.vy, packet.vz);
	clients[_id].do_send(sizeof(packet), &packet);
}

//데미지 깍는 곳
void send_change_hp(int _s_id)
{
	SC_DAMAGE_CHANGE packet;
	packet.size = sizeof(packet);
	packet.type = SC_PLAYER_DAMAGE;
	packet.damaged_id = _s_id;
	packet.hp = clients[_s_id]._hp;
	clients[_s_id].do_send(sizeof(packet), &packet);
}

void send_ready_packet(int _s_id)
{
	SC_ACCEPT_READY packet;
	packet.size = sizeof(packet);
	packet.type = SC_ALL_READY;
	packet.ingame = true;
	clients[_s_id].do_send(sizeof(packet), &packet);
}


//void send_damage_packet(int _s_id)
//{
//	SC_ATTACK_PLAYER packet;
//	packet.size = sizeof(packet);
//	packet.type = SC_DAMAGED;
//	packet.clientid = _s_id;
//	packet.sx = clients[_s_id].s_x;
//	packet.sy = clients[_s_id].s_y;
//	packet.sz = clients[_s_id].s_z;
//	packet.ex = clients[_s_id].e_x;
//	packet.ey = clients[_s_id].e_y;
//	packet.ez = clients[_s_id].e_z;
//	clients[_s_id].do_send(sizeof(packet), &packet);
//}