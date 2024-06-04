﻿//----------------------------------------------------------------------------------------------------------------------------------------------
// GameServer.cpp 파일
//----------------------------------------------------------------------------------------------------------------------------------------------

#include "pch.h"
#include "CorePch.h"
#include "CLIENT.h"
#include "Overlap.h"
#include "LockQueue.h"
#include "EscapeObject.h"
#include "DB.h"

;

HANDLE g_h_iocp;
HANDLE g_timer;
SOCKET sever_socket;
LockQueue<timer_ev> timer_q;
array <CLIENT, MAX_USER> clients;
array<EscapeObject, 11> objects;
condition_variable cv;
atomic<int> ready_count = 0;
atomic<int> ingamecount = 0;
using namespace chrono;

void ev_timer();
int get_id();
void send_select_character_type_packet(int _s_id);
void send_login_ok_packet(int _s_id);
void send_move_packet(int _id, int target);
void send_change_hp(int _s_id);
void Disconnect(int _s_id);
void send_ready_packet(int _s_id);
void send_endgame_packet(int _s_id);
void player_heal(int s_id);
void player_reload_weapon(int s_id);
void send_myitem_count_packet(int _s_id);
void send_item_packet(int _s_id, int item_index);
void send_myitem_packet(int _s_id);
void worker_thread();
void timer();
void send_delta_time_to_clients(double deltaTime);
void send_travel_ready_packet(int _s_id);
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
	for (int i = 0; i < MAX_OBJ; ++i)
		objects[i].ob_id = i;
	objects[0].x = 1710.f;
	objects[0].y = -1080.f;
	objects[0].z = 120.f;

	objects[1].x = 1800.f;
	objects[1].y = -570.f;
	objects[1].z = 80.f;

	
	objects[2].x = 2280.f;
	objects[2].y = -630.f;
	objects[2].z = 100.f;

	objects[3].x = 2110.f;
	objects[3].y = -1080.f;
	objects[3].z = 120.f;
	save_data();
	vector <thread> worker_threads;
	thread servertherad{ ev_timer };
	thread TimerThread{ timer };
	for (int i = 0; i < 16; ++i)
		worker_threads.emplace_back(worker_thread);

	for (auto& th : worker_threads)
		th.join();
	if (servertherad.joinable())
		servertherad.join();
	TimerThread.join();
	for (auto& cl : clients) {
		if (ST_INGAME == cl._state)
			Disconnect(cl._s_id);
	}
	closesocket(sever_socket);
	WSACleanup();
}


void Player_Event(int target, int player_id, IO_type type)
{
	Overlap* exp_over = new Overlap;
	exp_over->_op = type;
	exp_over->_target = player_id;
	PostQueuedCompletionStatus(g_h_iocp, 1, target, &exp_over->_wsa_over);
}

//타이머 큐 등록
void Timer_Event(int np_s_id, int user_id, EVENT_TYPE ev, std::chrono::milliseconds ms)
{
	timer_ev order;
	order.this_id = np_s_id;
	order.target_id = user_id;
	order.order = ev;
	order.start_t = chrono::system_clock::now() + ms;
	timer_q.Push(order);
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
	strcpy_s(packet.cid, clients[_s_id].name);
	cout << "_s_id" << _s_id << endl;

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

void Disconnect(int _s_id)
{
	CLIENT& cl = clients[_s_id];
	clients[_s_id].state_lock.lock();
	clients[_s_id]._state = ST_FREE;
	clients[_s_id].state_lock.unlock();
	closesocket(clients[_s_id]._socket);
	cout << "------------연결 종료------------" << endl;
}

void process_packet(int s_id, char* p)
{
	unsigned char packet_type = p[1];
	switch (packet_type) {
	case CS_LOGIN: {
		CS_LOGIN_PACKET* packet = reinterpret_cast<CS_LOGIN_PACKET*>(p);

		CLIENT& cl = clients[s_id];
		cout << "[Recv login] ID :" << packet->id << ", PASSWORD : " << packet->pw << endl;
		cl.state_lock.lock();
		cl._state = ST_INGAME;
		cl.state_lock.unlock();
		cout << "cl.sid : " << cl._s_id << endl;
		strcpy_s(cl.name, packet->id);
		cout << "czc : " << cl.name << endl;
		send_login_ok_packet(cl._s_id);
		cout << "플레이어[" << s_id << "]" << " 로그인 성공" << endl;

		break;

	}
	case CS_SELECT_CHAR: {

		CS_SELECT_CHARACTER* packet = reinterpret_cast<CS_SELECT_CHARACTER*>(p);
		CLIENT& cl = clients[packet->id];
		cout << "cl.sid?= " << packet->id << ", 00 , " << cl._s_id << endl;
		cl.x = packet->x;
		cl.y = packet->y;
		cl.z = packet->z;
		cl.p_type = packet->p_type;
		cl.connected = true;
		ingamecount++;
		send_select_character_type_packet(cl._s_id);

		
		if (ingamecount >= 2)
		{
			for (auto& player : clients) {
				if (ST_INGAME != player._state)
					continue;

				send_ready_packet(player._s_id);
				cout << "보낼 플레이어" << player._s_id << endl;

			}
			cout << "cl._s_id : " << cl._s_id << ", 131 ,, " << cl.p_type << endl;
			
		}
		break;
	}
	case CS_MOVE_Packet: {
		CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(p);
	//	cout << "packet_size = movepacket" << sizeof(packet) << endl;
		CLIENT& cl = clients[packet->id];
		cl.x = packet->x;
		cl.y = packet->y;
		cl.z = packet->z;
		cl.Yaw = packet->yaw;
		cl.VX = packet->vx;
		cl.VY = packet->vy;
		cl.VZ = packet->vz;
		cl.Max_Speed = packet->Max_speed;
		
//		cout << "hp : " << cl._hp << endl;
		for (auto& other : clients) {
			if (other._s_id == s_id)
				continue;
			if (ST_INGAME != other._state)
				continue;
			send_move_packet(other._s_id, cl._s_id);
		}
		break;
	}
	case CS_SELECT_WEP:
	{
		CS_SELECT_WEAPO* packet = reinterpret_cast<CS_SELECT_WEAPO*>(p);
		CLIENT& cl = clients[packet->id];
		cl.w_type = packet->weapon_type;
		cl.selectweapon = packet->bselectwep;
		//if(weapon type에 따라서 bullet 개수 적용)
		cout << "플레이어 : " << cl._s_id << "무기 타입" << cl.w_type << endl;
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
			packet.bselectwep = cl.selectweapon;
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_READY: {
		CS_READY_PACKET* packet = reinterpret_cast<CS_READY_PACKET*>(p);
		CLIENT& cl = clients[s_id];
		ready_count++;
		cout << "ready_count : " << ready_count << endl;
		if (ready_count >= 2)
		{
			for (auto& player : clients) {
				if (ST_INGAME != player._state)
					continue;
				send_travel_ready_packet(player._s_id);
				cout << "보낼 플레이어" << player._s_id << endl;
			}
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
		cl.pitch0 = packet->pitch0;
		cl.yaw0 = packet->yaw0;
		cl.roll0 = packet->roll0;
		cl.pitch1 = packet->pitch1;
		cl.yaw1 = packet->yaw1;
		cl.roll1 = packet->roll1;
		cl.pitch2 = packet->pitch2;
		cl.yaw2 = packet->yaw2;
		cl.roll2 = packet->roll2;
		cl.pitch3 = packet->pitch3;
		cl.yaw3 = packet->yaw3;
		cl.roll3 = packet->roll3;
		cl.pitch4 = packet->pitch4;
		cl.yaw4 = packet->yaw4;
		cl.roll4 = packet->roll4;
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
			packet.pitch0 = cl.pitch0;
			packet.yaw0 = cl.yaw0;
			packet.roll0 = cl.roll0;
			packet.pitch1 = cl.pitch1;
			packet.yaw1 = cl.yaw1;
			packet.roll1 = cl.roll1;
			packet.pitch2 = cl.pitch2;
			packet.yaw2 = cl.yaw2;
			packet.roll2 = cl.roll2;
			packet.pitch3 = cl.pitch3;
			packet.yaw3 = cl.yaw3;
			packet.roll3 = cl.roll3;
			packet.pitch4 = cl.pitch4;
			packet.yaw4 = cl.yaw4;
			packet.roll4 = cl.roll4;
	
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);

		}
		break;
	}
	case CS_START_GAME: {
		CS_START_GAME_PACKET* packet = reinterpret_cast<CS_START_GAME_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cout << "몇명 들어옴 : " << ingamecount << endl;
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
			packet.size = sizeof(packet);
			packet.type = SC_OTHER_PLAYER;
			packet.x = cl.x;
			packet.y = cl.y;
			packet.z = cl.z;
			packet.yaw = cl.Yaw;
			packet.hp = 100.f;
			packet.Max_speed = cl.Max_Speed;
			packet.p_type = cl.p_type;
			printf_s("[Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);
		}

		// 새로 접속한 플레이어에게 주위 객체 정보를 보낸다
		/*for (auto& other : clients) {
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
		}*/
		
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
		cl.wtype = packet->wep_type;
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
			packet.wep_type = cl.wtype;
			other.do_send(sizeof(packet), &packet);

		}
		break;

	}
	case CS_NiAGARA: {
		//cout << "나이아가라 들어옴?" << endl;
		CS_NIAGARA_SYNC_PACKET* packet = reinterpret_cast<CS_NIAGARA_SYNC_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cl.p_type = packet->playertype;
		cl.num = packet->num;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_NIAGARA_SYNC_PACKET packet;
			packet.id = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_NiAGARA;
			packet.playertype = cl.p_type;
			packet.num = cl.num;
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);

		}
		break;
	}
	case CS_NiAGARA_CANCEL: {
		CS_NIAGARA_CANCEL_PACKET* packet = reinterpret_cast<CS_NIAGARA_CANCEL_PACKET*>(p);
		CLIENT& cl = clients[packet->id];

		cl.bCancel = packet->cancel;
		cl.num = packet->num;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_NIAGARA_CANCEL_PACKET packet;
			packet.id = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_NiAGARA_CANCEL;
			packet.cancel = cl.bCancel;
			packet.num = cl.num;
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);

		}
		break;
	}
	case CS_NiAGARA_CH1: {
		CS_NIAGARA_PACKETCH1* packet = reinterpret_cast<CS_NIAGARA_PACKETCH1*>(p);
		CLIENT& cl = clients[packet->id];
		cl.p_type = packet->playertype;
		cl.x = packet->x;
		cl.y = packet->y;
		cl.z = packet->z;
		cl.num = packet->num;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_NIAGARA_PACKETCH1 packet;
			packet.id = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_NiAGARA_CH1;
			packet.playertype = cl.p_type;
			packet.x = cl.x;
			packet.y = cl.y;
			packet.z = cl.z;
			packet.num = cl.num;
			other.do_send(sizeof(packet), &packet);

		}
		break;
	}
	case CS_SIGNAl: {
		cout << "aaaaa들어옴?" << endl;
		CS_SIGNAL_PACKET* packet = reinterpret_cast<CS_SIGNAL_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cl.num = packet->num;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_SIGNAL_PACKET packet;
			packet.id = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_SIGNAL;
			packet.num = cl.num;
			other.do_send(sizeof(packet), &packet);

		}
		break;
	}
	case CS_END_GAME: {
		CS_END_GAME_PACKET* packet = reinterpret_cast<CS_END_GAME_PACKET*>(p);
		CLIENT& cl = clients[packet->id];

		cout << "누가 이김 " << packet->id << endl;
		for (auto& other : clients) {
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_END_GAME_PACKET packet;

			packet.id = cl._s_id;
			packet.winnerid = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_END_GAME;
			packet.bEND = true;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_GETITEM: {
		CS_ITEM_PACKET* packet = reinterpret_cast<CS_ITEM_PACKET*>(p);

		CLIENT& cl = clients[packet->id];
		cl.myItemCount = packet->itemCount;
		send_myitem_packet(cl._s_id);

		cout << "내가 획득한 아이템 개수" << cl._s_id << " : " << cl.myItemCount << endl;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			SC_ITEM_ACQUIRE_PACKET packet;
			packet.size = sizeof(packet);
			packet.type = SC_ITEM_ACQUIRE;
			strcpy_s(packet.cid, cl.name);
			packet.id = cl._s_id;
			packet.acquireid = cl._s_id;
			packet.itemCount = cl.myItemCount;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_STOP_ANIM: {
		CS_STOP_ANIM_PACKET* packet = reinterpret_cast<CS_STOP_ANIM_PACKET*>(p);
		cout << "들어옴 : " << packet->id << endl;
		CLIENT& cl = clients[packet->id];
		cl.bStopAnim = true;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_STOP_ANIM_PACKET packet;


			packet.size = sizeof(packet);
			packet.type = SC_STOP_ANIM;
			packet.id = cl._s_id;
			packet.bStopAnim = cl.bStopAnim;
			other.do_send(sizeof(packet), &packet);
		}

		break;
	}
	case CS_REMOVE_ITEM: {
		CS_REMOVE_ITEM_PACKET* packet = reinterpret_cast<CS_REMOVE_ITEM_PACKET*>(p);
		CLIENT& cl = clients[s_id];
		cout << "itemid : " << packet->itemid << endl;
		int itemid = packet->itemid;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_REMOVE_ITEM_PACKET packet;
			packet.itemid = itemid;
			packet.size = sizeof(packet);
			packet.type = SC_REMOVE_ITEM;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_INCREASE_COUNT: {
		CS_INCREASE_ITEM_PACKET* packet = reinterpret_cast<CS_INCREASE_ITEM_PACKET*>(p);
		CLIENT& cl = clients[packet->Increaseid];
		cl.myItemCount = packet->itemCount;
		send_myitem_count_packet(cl._s_id);

		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_INCREASE_ITEM_PACKET packet;
			packet.Increaseid = cl._s_id;
			strcpy_s(packet.cid, cl.name);
			packet.itemCount = cl.myItemCount;
			packet.size = sizeof(packet);
			packet.type = SC_INCREASE_COUNT;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_DECREASE_COUNT: {
		CS_INCREASE_ITEM_PACKET* packet = reinterpret_cast<CS_INCREASE_ITEM_PACKET*>(p);
		CLIENT& cl = clients[packet->Increaseid];
		cl.myItemCount = packet->itemCount;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_INCREASE_ITEM_PACKET packet;
			packet.Increaseid = cl._s_id;
			strcpy_s(packet.cid, cl.name);
			packet.itemCount = cl.myItemCount;
			packet.size = sizeof(packet);
			packet.type = SC_INCREASE_COUNT;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_ITEM_INFO: {
		CS_ITEM_INFO_PACKET* packet = reinterpret_cast<CS_ITEM_INFO_PACKET*> (p);
		CLIENT& cl = clients[s_id];

		send_item_packet(cl._s_id, packet->objid);
		break;
	}
	case CS_RELOAD: {
		CS_RELOAD_PACKET* packet = reinterpret_cast<CS_RELOAD_PACKET*>(p);
		CLIENT& cl = clients[packet->id];

		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_RELOAD_PACKET packet;
			packet.size = sizeof(packet);
			packet.type = SC_RELOAD;
			packet.id = cl._s_id;
			packet.bReload = true;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_ITEM_ANIM: {
		CS_ITEM_ANIM_PACKET* packet = reinterpret_cast<CS_ITEM_ANIM_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cl.itemAnimNum = packet->num;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_ITEM_ANIM_PACKET packet;
			packet.size = sizeof(packet);
			packet.type = SC_ITEM_ANIM;
			packet.id = cl._s_id;
			packet.num = cl.itemAnimNum;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_REMOVE_WEAPON: {
		CS_REMOVE_WEAPON_PACKET* packet = reinterpret_cast<CS_REMOVE_WEAPON_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cl.bGetWeapon = packet->bWeapon;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_REMOVE_WEAPON_PACKET packet;
			packet.size = sizeof(packet);
			packet.type = SC_REMOVE_WEAPON;
			packet.id = cl._s_id;
			packet.bWeapon = cl.bGetWeapon;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_CH2_SKILL: {
		SC_CH2_SKILL_PACKET* packet = reinterpret_cast<SC_CH2_SKILL_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cl.p_type = packet->p_type;
		
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			SC_CH2_SKILL_PACKET packet;
			packet.size = sizeof(packet);
			packet.type = SC_CH2_SKILL;
			packet.id = cl._s_id;
			packet.p_type = cl.p_type;
			packet.bfinish = true;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_MOPP:{
		CS_MOPP_PACKET* packet = reinterpret_cast<CS_MOPP_PACKET*>(p);
		CLIENT& cl = clients[s_id];
		cout << "itemid : " << packet->itemid << endl;
		int itemid = packet->itemid;
		int mopptype =  packet->mopptype;
		cout << "mopptype : " << mopptype << endl;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_MOPP_PACKET packet;
			packet.itemid = itemid;
			packet.size = sizeof(packet);
			packet.type = SC_MOPP;
			packet.mopptype = mopptype;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_HP: {
		CS_DAMAGE_PACKET* packet = reinterpret_cast<CS_DAMAGE_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cl._hp = packet->hp;
		cout << "hp : " << cl._hp << endl;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			SC_DAMAGE_CHANGE packet;
			packet.size = sizeof(packet);
			packet.type = SC_HP;
			packet.id = cl._s_id;
			packet.hp = cl._hp;
			other.do_send(sizeof(packet), &packet);
		}
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
			cout << "n__s_id : " << n__s_id << endl;
			if (-1 == n__s_id) {
				cout << "user over.\n";
			}
			else {
				CLIENT& cl = clients[n__s_id];
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


			break;
		}
		case IO_RELOAD_WEAPON: {
			// 플레이어의 탄창을 늘려주고
			// if(탄창이 max count 보다 작다면) 
				//player_reload_weapon(_s_id);
			// send_packet(_S_id);
			break;
		}
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
	clients[_id].do_send(sizeof(packet), &packet);
}
void player_heal(int s_id)
{
	if (clients[s_id]._hp < clients[s_id]._max_hp) 
		Timer_Event(s_id, s_id, ET_HEAL, 1s);
	
}
void player_reload_weapon(int s_id)
{
	if (clients[s_id]._bulletcount < clients[s_id]._max_bulletcount)
		Timer_Event(s_id, s_id, ET_RELOAD, 100ms);

}
//데미지 깍는 곳
void send_change_hp(int _s_id)
{
	SC_DAMAGE_CHANGE packet;
	packet.size = sizeof(packet);
	packet.type = SC_HP;
	packet.id = _s_id;
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
void send_travel_ready_packet(int _s_id)
{
	SC_TRAVEL_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_TRAVLE;
	packet.ingame = true;
	clients[_s_id].do_send(sizeof(packet), &packet);
}

void send_endgame_packet(int _s_id)
{
	CS_END_GAME_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_END_GAME;
	packet.id = _s_id;
	packet.bEND = true;
	clients[_s_id].do_send(sizeof(packet), &packet);
}
void send_myitem_packet(int _s_id)
{
	SC_MY_ITEM_COUNT packet;
	packet.size = sizeof(packet);
	packet.type = SC_MYITEM_COUNT;
	packet.id = _s_id;
	packet.MyITEMCount = clients[_s_id].myItemCount;
	clients[_s_id].do_send(sizeof(packet), &packet);
}
void send_myitem_count_packet(int _s_id)
{
	SC_MYNEW_ITEM_COUNT packet;
	packet.size = sizeof(packet);
	packet.type = SC_MYNEW_COUNT;
	packet.id = _s_id;
	packet.MyITEMCount = clients[_s_id].myItemCount;
	clients[_s_id].do_send(sizeof(packet), &packet);
}


void send_item_packet(int _s_id, int item_index)
{
	SC_ITEM_PACKET packet;

	packet.type = SC_ITEM;
	packet.size = sizeof(packet);

	packet.x = objects[item_index].x;
	packet.y = objects[item_index].y;
	packet.z = objects[item_index].z;
	packet.id = objects[item_index].ob_id; // 아이템 ID 설정

	clients[_s_id].do_send(sizeof(packet), &packet);

}

void ev_timer()
{
	while (true)
	{
		//cout << "adad" << endl;
		while (!timer_q.Empty()) {
			
			timer_ev order;
			timer_q.WaitPop(order);
			auto t = order.start_t - chrono::system_clock::now();
			int s_id = order.this_id;
			if (clients[s_id]._state != ST_INGAME) continue;
			if (clients[s_id]._is_active == false) continue;
			if (order.start_t <= chrono::system_clock::now()) {
				if (order.order == ET_HEAL) {
					Player_Event(s_id, order.target_id, IO_HEAL_HP);
					this_thread::sleep_for(100ms);
				}
				else if (order.order == ET_RELOAD) {
					Player_Event(s_id, order.target_id, IO_RELOAD_WEAPON);
					this_thread::sleep_for(100ms);
				}
			}
			else {
				timer_q.Push(order);
				this_thread::sleep_for(10ms);
			}
		}
	}
	

}

void timer()
{
	auto prev_time = high_resolution_clock::now();
	while (true) {
		auto current_time = high_resolution_clock::now();
		duration<double> delta = current_time - prev_time;
		prev_time = current_time;
		double delta_time = delta.count();
		
		send_delta_time_to_clients(delta_time);

		this_thread::sleep_for(milliseconds(1000 / 60));
	}
}

void send_delta_time_to_clients(double deltaTime) {
	SC_DELTA_TIME_PACKET packet;
	packet.time = deltaTime;
	packet.size = sizeof(packet);
	packet.type = SC_DELTA;
	//cout << "dt" << packet.time << endl;
	for (auto& client : clients) {
		client.state_lock.lock();
		if (client._state == ST_INGAME) {
			client.do_send(packet.size, &packet);
			//std::cout << "Send delta time " << deltaTime << " to client " << client._s_id << std::endl;
		}
		client.state_lock.unlock();
	}
}

