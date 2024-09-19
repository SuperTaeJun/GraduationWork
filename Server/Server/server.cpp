#include "pch.h"

//게임룸
//class  GameRoom {
//public:
//	vector<int> players;  // 게임룸 내 클라이언트 ID 목록
//public:
//	void clear() {
//		players.clear();
//	}
//};
//vector<GameRoom> gameRooms(4); // 게임룸 컨테이너 

HANDLE g_h_iocp;
HANDLE g_timer;
SOCKET sever_socket;
condition_variable cv;
LockQueue<timer_ev> timer_q;
mutex mtx;



//void worker_thread();
//void Disconnect(int _s_id);

//void ev_timer();
//int get_id();
//void send_select_character_type_packet(int _s_id);
//void send_login_ok_packet(int _s_id);
//void send_login_fail_packet(int _s_id, int failType);
//void send_move_packet(int _id, int target);
//void send_change_hp(int _s_id);
//void send_ready_packet(int _s_id);
//void send_travel_ready_packet(int _s_id);
//void send_endgame_packet(int _s_id);
//void send_myitem_count_packet(int _s_id);
//void send_item_packet(int _s_id, int item_index);
//void send_myitem_packet(int _s_id);
//void send_bullet_wall(int _s_id, int wall_index);
//void SendLobbyPacket(int clientId, bool bintoRoom);

int main()
{
	wcout.imbue(locale("korean"));
	IOCPServer server;

	server.Initialize();
	server.Run();
	//WSADATA WSAData;
	//::WSAStartup(MAKEWORD(2, 2), &WSAData);
	//sever_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	//SOCKADDR_IN server_addr;
	//ZeroMemory(&server_addr, sizeof(server_addr));
	//server_addr.sin_family = AF_INET;
	//server_addr.sin_port = htons(SERVER_PORT);
	//server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	//bind(sever_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	//listen(sever_socket, SOMAXCONN);
	//cout << "서버 시작" << endl;
	//g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	//CreateIoCompletionPort(reinterpret_cast<HANDLE>(sever_socket), g_h_iocp, 0, 0);

	//SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	//char   accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	//Overlap   accept_ex;
	//*(reinterpret_cast<SOCKET*>(&accept_ex._net_buf)) = c_socket;
	//ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
	//accept_ex._op = IO_ACCEPT;

	//AcceptEx(sever_socket, c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
	//	sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);


	////cout << gameRooms.size() << endl;
	//for (int i = 0; i < MAX_USER; ++i)
	//	clients[i]._s_id = i;

	//g_timer = CreateEvent(NULL, FALSE, FALSE, NULL);
	//vector <thread> worker_threads;
	////thread servertherad{ ev_timer };

	///*for (int i = 0; i < 16; ++i)
	//	worker_threads.emplace_back(worker_thread);

	//for (auto& th : worker_threads)
	//	th.join();*/

	///*if (servertherad.joinable())
	//	servertherad.join();*/
	///*for (auto& cl : clients) {
	//	if (ST_INGAME == cl._state)
	//		Disconnect(cl._s_id);
	//}*/
	//closesocket(sever_socket);
	//WSACleanup();
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

//해제
//void Disconnect(int _s_id)
//{
//	/*CLIENT& cl = clients[_s_id];
//	clients[_s_id].state_lock.lock();
//	clients[_s_id]._state = ST_FREE;
//	clients[_s_id].state_lock.unlock();
//	closesocket(clients[_s_id]._socket);
//	cout << "------------연결 종료------------" << endl;*/
//}

//새로운 id(인덱스) 할당
//int get_id()
//{
//	static int g_id = 0;
//
//	for (int i = 0; i < MAX_USER; ++i) {
//		clients[i].state_lock.lock();
//		if (ST_FREE == clients[i]._state) {
//			clients[i]._state = ST_ACCEPT;
//			clients[i].state_lock.unlock();
//			return i;
//		}
//		else clients[i].state_lock.unlock();
//	}
//	cout << "Maximum Number of Clients Overflow!!\n";
//	return -1;
//}




// 클라이언트를 지정된 게임룸에 매칭
//void assignClientToRoom(int clientId, int roomNum) {
//	unique_lock<mutex> lock(mtx);
//
//	// 지정된 방 번호가 유효한지 확인
//	if (roomNum < 0 || roomNum >= gameRooms.size()) {
//		cout << "Invalid room number: " << roomNum << endl;
//		return;
//	}
//
//	// 클라이언트를 지정된 방에 추가
//	gameRooms[roomNum].players.push_back(clientId);
//	clients[clientId].currentRoom = roomNum;
//	cout << "Client " << clientId << " assigned to room " << roomNum << endl;
//
//}
//void CanClientToRoom(int clientId, int roomNum) {
//	unique_lock<mutex> lock(mtx);
//
//	// 지정된 방 번호가 유효한지 확인
//	if (roomNum < 0 || roomNum >= gameRooms.size()) {
//		cout << "Invalid room number: " << roomNum << endl;
//		return;
//	}
//
//	// 지정된 방이 가득 찼는지 확인
//	if (gameRooms[roomNum].players.size() >= ENTER_CLIENT) {  // 방의 최대 인원수는 2명
//		cout << "Room " << roomNum << " is full." << endl;
//		SendLobbyPacket(clientId, false);
//		return;
//	}
//	else
//	{
//		SendLobbyPacket(clientId, true);
//	}
//}


////패킷 판별
//void process_packet(int s_id, unsigned char* p)
//{
//	unsigned char packet_type = p[1];
//	//CLIENT& cl = clients[s_id];
//	//cout << "user : " << s_id << "packet type :" << to_string(packet_type) << endl;
//	switch (packet_type) {
//	case CS_LOGIN: {
//		CS_LOGIN_PACKET* packet = reinterpret_cast<CS_LOGIN_PACKET*>(p);
//		CLIENT& cl = clients[s_id];
//		
//		cout << "[Recv login] ID :" << packet->id << ", PASSWORD : " << packet->pw << endl;
//		if (DB_odbc(packet->id, packet->pw))
//		{
//			cl.state_lock.lock();
//			cl._state = ST_INGAME;
//			cl.state_lock.unlock();
//			strcpy_s(cl.name, packet->id);
//			strcpy_s(cl._pw, packet->pw);
//			cl.bLogin = true;
//			send_login_ok_packet(cl._s_id);
//			cout << "플레이어[" << s_id << "]" << " 로그인 성공" << endl;
//		}
//		else
//		{
//			if (DB_id(packet->id) == true) {
//				cout << "플레이어[" << s_id << "]" << " 잘못된 비번" << endl;
//				send_login_fail_packet(s_id, WRONG_PW);
//				break;
//			}
//			else {
//				cout << "플레이어[" << s_id << "]" << " 잘못된 아이디" << endl;
//				send_login_fail_packet(s_id, WRONG_ID);
//				break;
//			}
//		}
//		break;
//	}
//	case CS_ACCOUNT: {
//		CS_ACCOUNT_PACKET* packet = reinterpret_cast<CS_ACCOUNT_PACKET*>(p);
//		CLIENT& cl = clients[s_id];
//		cout << "[Account login] ID :" << packet->id << ", PASSWORD : " << packet->pw << endl;
//		if (DB_id(packet->id) == true) {
//			cout << "플레이어[" << s_id << "] 계정 생성 실패 - " << "중복된 아이디" << endl;
//			send_login_fail_packet(s_id, OVERLAP_ID);
//		}
//		else
//		{
//			save_data(packet->id, packet->pw);
//			send_login_fail_packet(s_id, CREATE_AC);
//			cout << "계정 생성 완료" << endl;
//		}
//		
//		break;
//	}
//	case CS_ROOM:
//	{
//		CS_LOBBY_PACKET* packet = reinterpret_cast<CS_LOBBY_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.currentRoom = packet->RoomNum;
//		//assignClientToRoom(cl._s_id, cl.currentRoom);
//		break;
//	}
//	case CS_HOVER:
//	{
//		CS_LOBBY_HOVER* packet = reinterpret_cast<CS_LOBBY_HOVER*>(p);
//		cout << "hover packet 들어옴 " << endl;
//		CLIENT& cl = clients[packet->id];
//		int RoomNum = packet->RoomNum;
//		//CanClientToRoom(cl._s_id, RoomNum);
//		break;
//	}
//	case CS_SELECT_CHAR: {
//
//		//CS_SELECT_CHARACTER* packet = reinterpret_cast<CS_SELECT_CHARACTER*>(p);
//		//CLIENT& cl = clients[packet->id];
//		//cl.x = packet->x;
//		//cl.y = packet->y;
//		//cl.z = packet->z;
//		//cl.p_type = packet->p_type;
//		//cl.connected = true;
//		////ingamecount++;
//		//send_select_character_type_packet(cl._s_id);
//
//		//if (ingamecount >= ENTER_CLIENT)
//		//{
//		//	for (auto& player : clients) {
//		//		if (ST_INGAME != player._state)
//		//			continue;
//		//		if (player.currentRoom != cl.currentRoom)
//		//			continue;
//		//		send_ready_packet(player._s_id);
//		//		
//
//		//	}
//		//}
//		break;
//	}
//	case CS_MOVE_Packet: {
//		CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.x = packet->x;
//		cl.y = packet->y;
//		cl.z = packet->z;
//		cl.Yaw = packet->yaw;
//		cl.VX = packet->vx;
//		cl.VY = packet->vy;
//		cl.VZ = packet->vz;
//		cl.Max_Speed = packet->Max_speed;
//		cl.AO_PITCH = packet->AO_pitch;
//		cl.AO_YAW = packet->AO_yaw;
//		cl.jumpType = packet->jumpType;
//		for (auto& other : clients) {
//			if (other._s_id == s_id)
//				continue;
//			if (ST_INGAME != other._state)
//				continue;
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			send_move_packet(other._s_id, cl._s_id);
//		}
//		break;
//	}
//	case CS_SELECT_WEP:
//	{
//		CS_SELECT_WEAPO* packet = reinterpret_cast<CS_SELECT_WEAPO*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.w_type = packet->weapon_type;
//		cl.selectweapon = packet->bselectwep;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			SC_SYNC_WEAPO packet;
//			packet.id = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_OTHER_WEAPO;
//			packet.weapon_type = cl.w_type;
//			packet.bselectwep = cl.selectweapon;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_READY: {
//		//CS_READY_PACKET* packet = reinterpret_cast<CS_READY_PACKET*>(p);
//		//CLIENT& cl = clients[packet->id];
//		////ready_count++;
//		//if (ready_count >= ENTER_CLIENT)
//		//{
//		//	for (auto& player : clients) {
//		//		if (ST_INGAME != player._state)
//		//			continue;
//		//		if (player.currentRoom != cl.currentRoom)
//		//			continue;
//		//		send_travel_ready_packet(player._s_id);
//		//	}
//		//	ready_count = 0;
//		//}
//		break;
//	}
//	case CS_SHOTGUN_BEAM: {
//		CS_SHOTGUN_BEAM_PACKET* packet = reinterpret_cast<CS_SHOTGUN_BEAM_PACKET*>(p);
//		CLIENT& cl = clients[packet->attackid];
//		cl.s_x = packet->sx;
//		cl.s_y = packet->sy;
//		cl.s_z = packet->sz;
//		//--------------------
//		cl.pitch0 = packet->pitch0;
//		cl.yaw0 = packet->yaw0;
//		cl.roll0 = packet->roll0;
//		cl.pitch1 = packet->pitch1;
//		cl.yaw1 = packet->yaw1;
//		cl.roll1 = packet->roll1;
//		cl.pitch2 = packet->pitch2;
//		cl.yaw2 = packet->yaw2;
//		cl.roll2 = packet->roll2;
//		cl.pitch3 = packet->pitch3;
//		cl.yaw3 = packet->yaw3;
//		cl.roll3 = packet->roll3;
//		cl.pitch4 = packet->pitch4;
//		cl.yaw4 = packet->yaw4;
//		cl.roll4 = packet->roll4;
//		//--------------------
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_SHOTGUN_BEAM_PACKET packet;
//			packet.attackid = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_SHOTGUN_BEAM;
//			packet.sx = cl.s_x;
//			packet.sy = cl.s_y;
//			packet.sz = cl.s_z;
//			packet.pitch0 = cl.pitch0;
//			packet.yaw0 = cl.yaw0;
//			packet.roll0 = cl.roll0;
//			packet.pitch1 = cl.pitch1;
//			packet.yaw1 = cl.yaw1;
//			packet.roll1 = cl.roll1;
//			packet.pitch2 = cl.pitch2;
//			packet.yaw2 = cl.yaw2;
//			packet.roll2 = cl.roll2;
//			packet.pitch3 = cl.pitch3;
//			packet.yaw3 = cl.yaw3;
//			packet.roll3 = cl.roll3;
//			packet.pitch4 = cl.pitch4;
//			packet.yaw4 = cl.yaw4;
//			packet.roll4 = cl.roll4;
//			other.do_send(sizeof(packet), &packet);
//
//		}
//		break;
//	}
//	case CS_START_GAME: {
//		CS_START_GAME_PACKET* packet = reinterpret_cast<CS_START_GAME_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom) continue;
//			SC_PLAYER_SYNC packet;
//			packet.id = cl._s_id;
//			strcpy_s(packet.name, cl.name);
//			packet.size = sizeof(packet);
//			packet.type = SC_OTHER_PLAYER;
//			packet.x = cl.x;
//			packet.y = cl.y;
//			packet.z = cl.z;
//			packet.yaw = cl.Yaw;
//			packet.Max_speed = cl.Max_Speed;
//			packet.p_type = cl.p_type;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_HIT_EFFECT: {
//		CS_EFFECT_PACKET* packet = reinterpret_cast<CS_EFFECT_PACKET*>(p);
//		CLIENT& cl = clients[packet->attack_id];
//		cl.s_x = packet->lx;
//		cl.s_y = packet->ly;
//		cl.s_z = packet->lz;
//		cl.Pitch = packet->r_pitch;
//		cl.Yaw = packet->r_yaw;
//		cl.Roll = packet->r_roll;
//		cl.wtype = packet->wep_type;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_EFFECT_PACKET packet;
//			packet.attack_id = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_EFFECT;
//			packet.lx = cl.s_x;
//			packet.ly = cl.s_y;
//			packet.lz = cl.s_z;
//			packet.r_pitch = cl.Pitch;
//			packet.r_yaw = cl.Yaw;
//			packet.r_roll = cl.Roll;
//			packet.wep_type = cl.wtype;
//			other.do_send(sizeof(packet), &packet);
//
//		}
//		break;
//
//	}
//	case CS_BOJOWEAPON: {
//		CS_BOJOWEAPON_PACKET* packet = reinterpret_cast<CS_BOJOWEAPON_PACKET*>(p);
//		CLIENT& cl = clients[packet->attack_id];
//		cl.s_x = packet->lx;
//		cl.s_y = packet->ly;
//		cl.s_z = packet->lz;
//		cl.Pitch = packet->r_pitch;
//		cl.Yaw = packet->r_yaw;
//		cl.Roll = packet->r_roll;
//		cl.wtype = packet->wep_type;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_BOJOWEAPON_PACKET packet;
//			packet.attack_id = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_BOJOWEAPON;
//			packet.lx = cl.s_x;
//			packet.ly = cl.s_y;
//			packet.lz = cl.s_z;
//			packet.r_pitch = cl.Pitch;
//			packet.r_yaw = cl.Yaw;
//			packet.r_roll = cl.Roll;
//			packet.wep_type = cl.wtype;
//			other.do_send(sizeof(packet), &packet);
//
//		}
//		break;
//	}
//	case CS_NiAGARA: {
//		CS_NIAGARA_SYNC_PACKET* packet = reinterpret_cast<CS_NIAGARA_SYNC_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.p_type = packet->playertype;
//		cl.num = packet->num;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_NIAGARA_SYNC_PACKET packet;
//			packet.id = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_NiAGARA;
//			packet.playertype = cl.p_type;
//			packet.num = cl.num;
//			other.do_send(sizeof(packet), &packet);
//
//		}
//		break;
//	}
//	case CS_NiAGARA_CANCEL: {
//		CS_NIAGARA_CANCEL_PACKET* packet = reinterpret_cast<CS_NIAGARA_CANCEL_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.bCancel = packet->cancel;
//		cl.num = packet->num;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_NIAGARA_CANCEL_PACKET packet;
//			packet.id = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_NiAGARA_CANCEL;
//			packet.cancel = cl.bCancel;
//			packet.num = cl.num;
//			other.do_send(sizeof(packet), &packet);
//
//		}
//		break;
//	}
//	case CS_NiAGARA_CH1: {
//		CS_NIAGARA_PACKETCH1* packet = reinterpret_cast<CS_NIAGARA_PACKETCH1*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.p_type = packet->playertype;
//		cl.x = packet->x;
//		cl.y = packet->y;
//		cl.z = packet->z;
//		cl.num = packet->num;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_NIAGARA_PACKETCH1 packet;
//			packet.id = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_NiAGARA_CH1;
//			packet.playertype = cl.p_type;
//			packet.x = cl.x;
//			packet.y = cl.y;
//			packet.z = cl.z;
//			packet.num = cl.num;
//			other.do_send(sizeof(packet), &packet);
//
//		}
//		break;
//	}
//	case CS_SIGNAl: {
//		CS_SIGNAL_PACKET* packet = reinterpret_cast<CS_SIGNAL_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.num = packet->num;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_SIGNAL_PACKET packet;
//			packet.id = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_SIGNAL;
//			packet.num = cl.num;
//			other.do_send(sizeof(packet), &packet);
//
//		}
//		break;
//	}
//	case CS_END_GAME: {
//		CS_END_GAME_PACKET* packet = reinterpret_cast<CS_END_GAME_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		int RoomNum = cl.currentRoom;
//		cout << "누가 이김 " << packet->id << endl;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			other.currentRoom = -1;
//			CS_END_GAME_PACKET packet;
//
//			packet.id = cl._s_id;
//			packet.winnerid = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_END_GAME;
//			packet.bEND = true;
//			other.do_send(sizeof(packet), &packet);
//		}
//		//gameRooms[RoomNum].clear();
//		break;
//	}
//	case CS_GETITEM: {
//		CS_ITEM_PACKET* packet = reinterpret_cast<CS_ITEM_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.myItemCount = packet->itemCount;
//		//send_myitem_packet(cl._s_id);
//
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			SC_ITEM_ACQUIRE_PACKET packet;
//			packet.size = sizeof(packet);
//			packet.type = SC_ITEM_ACQUIRE;
//			strcpy_s(packet.cid, cl.name);
//			packet.id = cl._s_id;
//			packet.acquireid = cl._s_id;
//			packet.itemCount = cl.myItemCount;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_ALIVE: {
//		CS_ALIVE_PACKET* packet = reinterpret_cast<CS_ALIVE_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.deadtype = packet->deadtype;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_ALIVE_PACKET packet;
//
//
//			packet.size = sizeof(packet);
//			packet.type = SC_ALIVE;
//			packet.id = cl._s_id;
//			packet.deadtype = cl.deadtype;
//			other.do_send(sizeof(packet), &packet);
//		}
//
//		break;
//	}
//	case CS_DISSOLVE: {
//		CS_DISSOLVE_PACKET* packet = reinterpret_cast<CS_DISSOLVE_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		if (packet->dissolve == 1) {
//			cout << "dissolve : " << packet->dissolve << endl;
//			cout << "id : " << packet->id << endl;
//		}
//		cl.dissolve = packet->dissolve;
//		for (auto& other : clients) {
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_DISSOLVE_PACKET packet;
//
//			packet.size = sizeof(packet);
//			packet.type = SC_DISSOLVE;
//			packet.id = cl._s_id;
//			packet.dissolve = cl.dissolve;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_REMOVE_ITEM: {
//		CS_REMOVE_ITEM_PACKET* packet = reinterpret_cast<CS_REMOVE_ITEM_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.myItemCount += 1;
//		int itemid = packet->itemid;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_REMOVE_ITEM_PACKET packet;
//			packet.itemid = itemid;
//			packet.size = sizeof(packet);
//			packet.type = SC_REMOVE_ITEM;
//			packet.id = cl._s_id;
//			//packet.itemcount = cl.myItemCount;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_INCREASE_COUNT: {
//		CS_INCREASE_ITEM_PACKET* packet = reinterpret_cast<CS_INCREASE_ITEM_PACKET*>(p);
//		CLIENT& cl = clients[packet->Increaseid];
//		cl.myItemCount += packet->itemCount;
//		send_myitem_count_packet(cl._s_id);
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_INCREASE_ITEM_PACKET packet;
//			packet.Increaseid = cl._s_id;
//			strcpy_s(packet.cid, cl.name);
//			packet.itemCount = cl.myItemCount;
//			packet.size = sizeof(packet);
//			packet.type = SC_INCREASE_COUNT;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_DECREASE_COUNT: {
//		CS_DECREASE_ITEM_PACKET* packet = reinterpret_cast<CS_DECREASE_ITEM_PACKET*>(p);
//		CLIENT& cl = clients[packet->Increaseid];
//		cl.myItemCount = packet->itemCount;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_DECREASE_ITEM_PACKET packet;
//			packet.Increaseid = cl._s_id;
//			strcpy_s(packet.cid, cl.name);
//			packet.itemCount = cl.myItemCount;
//			packet.size = sizeof(packet);
//			packet.type = SC_DECREASE;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_ITEM_INFO: {
//		CS_ITEM_INFO_PACKET* packet = reinterpret_cast<CS_ITEM_INFO_PACKET*> (p);
//		CLIENT& cl = clients[s_id];
//		//send_item_packet(cl._s_id, packet->objid);
//		break;
//	}
//	case CS_BULLET_WALL: {
//		CS_WALL_PACKET* packet = reinterpret_cast<CS_WALL_PACKET*> (p);
//		CLIENT& cl = clients[packet->id];
//		walls[packet->wall_id].ob_id = packet->wall_id;
//		walls[packet->wall_id].x = packet->lx;
//		walls[packet->wall_id].y = packet->ly;
//		walls[packet->wall_id].z = packet->lz;
//		walls[packet->wall_id].roll = packet->r_roll;
//		walls[packet->wall_id].pitch = packet->r_pitch;
//		walls[packet->wall_id].yaw = packet->r_yaw;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			send_bullet_wall(other._s_id, packet->wall_id);
//		}
//		break;
//	}
//	case CS_RELOAD: {
//		CS_RELOAD_PACKET* packet = reinterpret_cast<CS_RELOAD_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.bReload = packet->bReload;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_RELOAD_PACKET packet;
//			packet.size = sizeof(packet);
//			packet.type = SC_RELOAD;
//			packet.id = cl._s_id;
//			packet.bReload = cl.bReload;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_ITEM_ANIM: {
//		CS_ITEM_ANIM_PACKET* packet = reinterpret_cast<CS_ITEM_ANIM_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.itemAnimNum = packet->num;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_ITEM_ANIM_PACKET packet;
//			packet.size = sizeof(packet);
//			packet.type = SC_ITEM_ANIM;
//			packet.id = cl._s_id;
//			packet.num = cl.itemAnimNum;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_REMOVE_WEAPON: {
//		CS_REMOVE_WEAPON_PACKET* packet = reinterpret_cast<CS_REMOVE_WEAPON_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.bGetWeapon = packet->bWeapon;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_REMOVE_WEAPON_PACKET packet;
//			packet.size = sizeof(packet);
//			packet.type = SC_REMOVE_WEAPON;
//			packet.id = cl._s_id;
//			packet.bWeapon = cl.bGetWeapon;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_CH2_SKILL: {
//		SC_CH2_SKILL_PACKET* packet = reinterpret_cast<SC_CH2_SKILL_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.p_type = packet->p_type;
//
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			SC_CH2_SKILL_PACKET packet;
//			packet.size = sizeof(packet);
//			packet.type = SC_CH2_SKILL;
//			packet.id = cl._s_id;
//			packet.p_type = cl.p_type;
//			packet.bfinish = true;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_DAMAGE: {
//		CS_DAMAGE_PACKET* packet = reinterpret_cast<CS_DAMAGE_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl._hp = packet->hp;
//		cl.bAlive = packet->bAlive;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom) continue;
//			SC_HP_CHANGE_PACKET packet;
//			packet.size = sizeof(packet);
//			packet.type = SC_HP_CHANGE;
//			packet.id = cl._s_id;
//			packet.HP = cl._hp;
//			packet.bAlive = cl.bAlive;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_BOJO_ANIM: {
//		CS_BOJO_ANIM_PACKET* packet = reinterpret_cast<CS_BOJO_ANIM_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.bojoanimtype = packet->bojoanimtype;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_BOJO_ANIM_PACKET packet;
//			packet.size = sizeof(packet);
//			packet.type = SC_BOJO_ANIM;
//			packet.id = cl._s_id;
//			packet.bojoanimtype = cl.bojoanimtype;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_MOPP: {
//		CS_MOPP_PACKET* packet = reinterpret_cast<CS_MOPP_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		int itemid = packet->itemid;
//		int mopptype = packet->mopptype;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_MOPP_PACKET packet;
//			packet.itemid = itemid;
//			packet.size = sizeof(packet);
//			packet.type = SC_MOPP;
//			packet.mopptype = mopptype;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_LIGHT: {
//		CS_LIGHT_ON_PACKET* packet = reinterpret_cast<CS_LIGHT_ON_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.bLightOn = packet->bLight;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_LIGHT_ON_PACKET packet;
//			packet.id = cl._s_id;
//			packet.size = sizeof(packet);
//			packet.type = SC_LIGHT;
//			packet.bLight = cl.bLightOn;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_RECHARGE: {
//		CS_RECHARGE_PACKET* packet = reinterpret_cast<CS_RECHARGE_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.bRecharge = packet->bRecharge;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_RECHARGE_PACKET packet;
//			packet.size = sizeof(packet);
//			packet.type = SC_RECHARGE;
//			packet.id = cl._s_id;
//			packet.bRecharge = cl.bRecharge;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	case CS_HIT_ANIM: {
//		CS_HIT_ANIM_PACKET* packet = reinterpret_cast<CS_HIT_ANIM_PACKET*>(p);
//		CLIENT& cl = clients[packet->id];
//		cl.bHitAnim = packet->bHitAnim;
//		for (auto& other : clients) {
//			if (other._s_id == cl._s_id) continue;
//			other.state_lock.lock();
//			if (ST_INGAME != other._state) {
//				other.state_lock.unlock();
//				continue;
//			}
//			else other.state_lock.unlock();
//			if (other.currentRoom != cl.currentRoom)
//				continue;
//			CS_HIT_ANIM_PACKET packet;
//			packet.size = sizeof(packet);
//			packet.type = SC_HIT_ANIM;
//			packet.id = cl._s_id;
//			packet.bHitAnim = cl.bHitAnim;
//			other.do_send(sizeof(packet), &packet);
//		}
//		break;
//	}
//	default:
//		cout << " 오류패킷타입 : " << p << endl;
//		break;
//	}
//}
//
//
////워크 쓰레드
//void worker_thread()
//{
//	while (1) {
//		DWORD num_byte;
//		LONG64 iocp_key;
//		WSAOVERLAPPED* p_over;
//		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);
//
//		int _s_id = static_cast<int>(iocp_key);
//		Overlap* exp_over = reinterpret_cast<Overlap*>(p_over);
//		if (FALSE == ret) {
//			int err_no = WSAGetLastError();
//			cout << "GQCS Error : ";
//			error_display(err_no);
//			cout << endl;
//			Disconnect(_s_id);
//			if (exp_over->_op == IO_SEND)
//				delete exp_over;
//			continue;
//		}
//
//		switch (exp_over->_op) {
//		case IO_RECV: {
//			if (num_byte == 0) {
//				cout << "연결종료" << endl;
//				Disconnect(_s_id);
//				continue;
//			}
//			CLIENT& cl = clients[_s_id];
//			int remain_data = num_byte + cl._prev_size;
//			unsigned char* packet_start = exp_over->_net_buf;
//			int packet_size = packet_start[0];
//
//			while (packet_size <= remain_data) {
//				process_packet(_s_id, packet_start);
//				remain_data -= packet_size;
//				packet_start += packet_size;
//				if (remain_data > 0) packet_size = packet_start[0];
//				else break;
//			}
//
//			if (0 < remain_data) {
//				cl._prev_size = remain_data;
//				memcpy(&exp_over->_net_buf, packet_start, remain_data);
//			}
//			if (remain_data == 0)
//				cl._prev_size = 0;
//			cl.do_recv();
//			break;
//		}
//		case IO_SEND: {
//			if (num_byte != exp_over->_wsa_buf.len) {
//				cout << "send 에러" << endl;
//				Disconnect(_s_id);
//			}
//			delete exp_over;
//			break;
//		}
//		case IO_ACCEPT: {
//			cout << "Accept Completed.\n";
//			SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->_net_buf));
//
//			int n__s_id = get_id();
//			cout << "n__s_id : " << n__s_id << endl;
//			if (-1 == n__s_id) {
//				cout << "user over.\n";
//			}
//			else {
//				CLIENT& cl = clients[n__s_id];
//				cl.state_lock.lock();
//				cl._s_id = n__s_id;
//				cl._state = ST_ACCEPT;
//				cl.state_lock.unlock();
//				cl._prev_size = 0;
//				cl._recv_over._op = IO_RECV;
//				cl._recv_over._wsa_buf.buf = reinterpret_cast<char*>(cl._recv_over._net_buf);
//				cl._recv_over._wsa_buf.len = sizeof(cl._recv_over._net_buf);
//				ZeroMemory(&cl._recv_over._wsa_over, sizeof(cl._recv_over._wsa_over));
//				cl._socket = c_socket;
//
//				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, n__s_id, 0);
//				cl.do_recv();
//			}
//
//			ZeroMemory(&exp_over->_wsa_over, sizeof(exp_over->_wsa_over));
//			c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
//			*(reinterpret_cast<SOCKET*>(exp_over->_net_buf)) = c_socket;
//			AcceptEx(sever_socket, c_socket, exp_over->_net_buf + 8, 0, sizeof(SOCKADDR_IN) + 16,
//				sizeof(SOCKADDR_IN) + 16, NULL, &exp_over->_wsa_over);
//
//
//			break;
//		}
//		}
//	}
//}




