﻿//----------------------------------------------------------------------------------------------------------------------------------------------
// GameServer.cpp 파일
//----------------------------------------------------------------------------------------------------------------------------------------------

#include "pch.h"
#include "CorePch.h"
#include "CLIENT.h"
#include "Overlap.h"
#include "LockQueue.h"
#include "EscapeObject.h"

;

HANDLE g_h_iocp;
HANDLE g_timer;
SOCKET sever_socket;
LockQueue<timer_ev> timer_q;
array <CLIENT, MAX_USER> clients;
array<EscapeObject, MAX_OBJ> objects;
condition_variable cv;
atomic<int> ready_count = 0;
atomic<int> ingamecount = 0;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

void ev_timer();
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
void send_endgame_packet(int _s_id);
void send_item_packet(int _s_id, int item_index);
void worker_thread();
// 랜덤 넘버 생성기 초기화
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(-5000.0f, 5000.0f);
std::uniform_real_distribution<float> disz(82.15f, 100.0f);

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
	for (int i = 0; i < MAX_OBJ; ++i) {
		objects[i].ob_id = i;
		objects[i].setRandomPosition(gen, dis, disz); // 랜덤한 좌표 설정
	}

	for (int i = 0; i < MAX_OBJ; ++i) {
		cout << "pos : " << objects[i].x << ", " << objects[i].z << endl;
	}

	g_timer = CreateEvent(NULL, FALSE, FALSE, NULL);
	vector <thread> worker_threads;
	thread servertherad{ ev_timer };

	for (int i = 0; i < 16; ++i)
		worker_threads.emplace_back(worker_thread);

	for (auto& th : worker_threads)
		th.join();
	if (servertherad.joinable())
		servertherad.join();
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

		strcpy_s(cl.name, packet->id);
		cout << "czc : " << cl.name << endl;
		send_login_ok_packet(cl._s_id);
		cout << "플레이어[" << s_id << "]" << " 로그인 성공" << endl;

		break;

	}
	case CS_SELECT_CHAR: {

		CS_SELECT_CHARACTER* packet = reinterpret_cast<CS_SELECT_CHARACTER*>(p);
		CLIENT& cl = clients[s_id];
		cl.x = packet->x;
		cl.y = packet->y;
		cl.z = packet->z;
		cl.p_type = packet->p_type;
		cl.connected = true;
		ingamecount++;
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
		cout << "몇명 들어옴 : " << ingamecount << endl;
		//m.unlock();
		if (ingamecount >= 2)
		{
			for (auto& player : clients) {
				if (ST_INGAME != player._state)
					continue;
				/*state_lock();*/
				send_ready_packet(player._s_id);
				cout << "보낼 플레이어" << player._s_id << endl;
				//m.unlock();
			}
		}
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
			packet.bselectwep = cl.selectweapon;
			//printf_s("[Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_READY:
	{
		CS_READY_PACKET* packet = reinterpret_cast<CS_READY_PACKET*>(p);
		CLIENT& cl = clients[s_id];
		ready_count++;
		cout << "ready_count" << ready_count << endl;
		if (ready_count >= 3)
		{
			for (auto& player : clients) {
				if (ST_INGAME != player._state)
					continue;
				//m.lock();
				send_ready_packet(player._s_id);
				cout << "보낼 플레이어" << player._s_id << endl;
				//m.unlock();
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
		cl.pitch5 = packet->pitch5;
		cl.yaw5 = packet->yaw5;
		cl.roll5 = packet->roll5;

		cl.pitch6 = packet->pitch6;
		cl.yaw6 = packet->yaw6;
		cl.roll6 = packet->roll6;
		cl.pitch7 = packet->pitch7;
		cl.yaw7 = packet->yaw7;
		cl.roll7 = packet->roll7;
		cl.pitch8 = packet->pitch8;
		cl.yaw8 = packet->yaw8;
		cl.roll8 = packet->roll8;
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
			packet.pitch5 = cl.pitch5;
			packet.yaw5 = cl.yaw5;
			packet.roll5 = cl.roll5;
			packet.pitch6 = cl.pitch6;
			packet.yaw6 = cl.yaw6;
			packet.roll6 = cl.roll6;
			packet.pitch7 = cl.pitch7;
			packet.yaw7 = cl.yaw7;
			packet.roll7 = cl.roll7;
			packet.pitch8 = cl.pitch8;
			packet.yaw8 = cl.yaw8;
			packet.roll8 = cl.roll8;
			/*packet.ex9 = cl.ex9;
			packet.ey9 = cl.ey9;
			packet.ez9 = cl.ez9;*/
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);

		}
		break;
	}
	case CS_START_GAME: {
		CS_START_GAME_PACKET* packet = reinterpret_cast<CS_START_GAME_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cout << "cl.sid" << cl._s_id << endl;
		//cl.state_lock.lock();
		for (int i = 0; i < 20; ++i) {
			send_item_packet(cl._s_id, i);
		}
		//cl.state_lock.unlock();
		break;
	}
	case CS_SHOTGUN_DAMAGED: {
		CS_SHOTGUN_DAMAGED_PACKET* packet = reinterpret_cast<CS_SHOTGUN_DAMAGED_PACKET*>(p);
		clients[packet->damaged_id].damage = packet->damage;
		send_change_hp(packet->damaged_id);
		clients[packet->damaged_id1].damage = packet->damage1;
		send_change_hp(packet->damaged_id1);
		clients[packet->damaged_id2].damage = packet->damage2;
		send_change_hp(packet->damaged_id2);

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
		cl.damage = packet->damage;
		send_change_hp(cl._s_id);

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
			//packet.weapon_type = cl.w_type;
		//printf_s("[Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			//	cout << "나이아가라" << endl;

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
			//packet.weapon_type = cl.w_type;
		//printf_s("[Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			//	cout << "나이아가라" << endl;

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
		cout << "내가 획득한 아이템 개수" << cl._s_id << " : " << cl.myItemCount << endl;
		for (auto& other : clients) {
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			SC_ITEM_ACQUIRE_PACKET packet;


			packet.size = sizeof(packet);
			packet.type = SC_ITEM_ACQUIRE;
			packet.id = cl._s_id;
			packet.acquireid = cl._s_id;
			packet.itemCount = cl.myItemCount;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_STOP_ANIM: {
		CS_STOP_ANIM_PACKET* packet = reinterpret_cast<CS_STOP_ANIM_PACKET*>(p);
		cout << "packet->id : " << packet->id << endl;
		CLIENT& cl = clients[packet->id];
		cl.bStopAnim = true;
		for (auto& other : clients) {
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
	packet.damage = clients[_s_id].damage;
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

void send_endgame_packet(int _s_id)
{
	CS_END_GAME_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_END_GAME;
	packet.id = _s_id;
	packet.bEND = true;
	clients[_s_id].do_send(sizeof(packet), &packet);
}

void send_item_packet(int _s_id, int item_index)
{
	SC_ITEM_PACKET packet;

	cout << "_s_id, packetsize" << _s_id << sizeof(packet) << endl;
	packet.type = SC_ITEM;
	packet.size = sizeof(packet);

	packet.x = objects[item_index].x;
	packet.y = objects[item_index].y;
	packet.z = objects[item_index].z;
	packet.id = objects[item_index].ob_id; // 아이템 ID 설정

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

void ev_timer()
{
	WaitForSingleObject(g_timer, INFINITE);
	{
		timer_q.Clear();
	}
	while (true) {
		timer_ev order;
		timer_q.WaitPop(order);
		auto t = order.start_t - chrono::system_clock::now();
		int s_id = order.this_id;
		if (clients[s_id]._state != ST_INGAME) continue;
		if (clients[s_id]._is_active == false) continue;
		if (order.start_t <= chrono::system_clock::now()) {
		}
		else {
			timer_q.Push(order);
			this_thread::sleep_for(10ms);
		}
	}

}