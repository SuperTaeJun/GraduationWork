#include "protocol.h"
#include <vector>
#include <list>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <thread>
#include <string>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <array>
#include <chrono>
#include <mutex>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <concurrent_priority_queue.h>
#include <iostream>
using namespace std;
#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")

enum EVENT_TYPE { ET_RELOAD, ET_HEAL };
enum IO_type
{
	IO_RECV,
	IO_SEND,
	IO_ACCEPT,
	IO_RELOAD_WEAPON,
	IO_HEAL_HP
};
enum CL_STATE { ST_FREE, ST_ACCEPT, ST_INGAME, ST_LOBBY };

void error_display(int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, 0);
	wcout << lpMsgBuf << endl;
	//while (true);
	LocalFree(lpMsgBuf);
}

class Overlap {
public:
	WSAOVERLAPPED   _wsa_over;
	IO_type         _op;
	WSABUF         _wsa_buf;
	unsigned		 char   _net_buf[2047];
	int            _target;
public:
	Overlap(IO_type _op, char num_bytes, void* mess) : _op(_op)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
		_wsa_buf.len = num_bytes;
		memcpy(_net_buf, mess, num_bytes);
	}

	Overlap(IO_type _op) : _op(_op) {}

	Overlap()
	{
		_op = IO_RECV;
	}

	~Overlap()
	{
	}
};

class CLIENT
{
public:
	int _s_id; //플레이어 배열 넘버
	char name[MAX_NAME_SIZE]; //플레이어 nick
	char _pw[MAX_NAME_SIZE];  // pw
	// 위치
	float	x;
	float	y;
	float	z;
	// 회전값
	// 아이템 획득 카운트
	int myItemCount;
	bool connected = false;
	bool selectweapon = false;
	float	Yaw;
	float	Pitch;
	float	Roll;
	float   Speed;
	// 에임 오프셋
	float AO_YAW, AO_PITCH;
	// 속도
	float VX;
	float VY;
	float VZ;
	float Max_Speed;

	float _hp; // 체력
	int damage;
	bool is_bone = false;
	bool bGetWeapon = false;
	bool bCancel;
	bool bEndGame = false;
	WeaponType w_type;
	PlayerType p_type;
	float s_x, s_y, s_z;
	float e_x, e_y, e_z;
	int wtype;
	//보조무기 애니메이션 상태
	int bojoanimtype;
	//--------------------
	//죽는 애니메이션 타입
	int deadtype;
	// 디졸브 타입
	int dissolve;
	unordered_set   <int>  viewlist; // 시야 안 오브젝트
	mutex vl;
	mutex hp_lock;
	mutex lua_lock;

	mutex state_lock;
	CL_STATE _state;
	atomic_bool  _is_active = false;

	int num;
	int itemAnimNum;
	atomic_int  _count;
	int      _type;
	//-------------
	float pitch0, yaw0, roll0;
	float pitch1, yaw1, roll1;
	float pitch2, yaw2, roll2;
	float pitch3, yaw3, roll3;
	float pitch4, yaw4, roll4;
	//-------------
	Overlap _recv_over;
	SOCKET  _socket;
	int      _prev_size;
	int      last_move_time;

public:
	CLIENT() : _state(ST_FREE), _prev_size(0)
	{
		_hp = 100.f;
		myItemCount = 0;
	}

	~CLIENT()
	{
		closesocket(_socket);
	}

	void do_recv()
	{
		DWORD recv_flag = 0;
		ZeroMemory(&_recv_over._wsa_over, sizeof(_recv_over._wsa_over));
		_recv_over._wsa_buf.buf = reinterpret_cast<char*>(_recv_over._net_buf + _prev_size);
		_recv_over._wsa_buf.len = sizeof(_recv_over._net_buf) - _prev_size;
		int ret = WSARecv(_socket, &_recv_over._wsa_buf, 1, 0, &recv_flag, &_recv_over._wsa_over, NULL);
		if (SOCKET_ERROR == ret) {
			int error_num = WSAGetLastError();
			if (ERROR_IO_PENDING != error_num)
				error_display(error_num);
		}
	}

	void do_send(int num_bytes, void* mess)
	{
		Overlap* ex_over = new Overlap(IO_SEND, num_bytes, mess);
		int ret = WSASend(_socket, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, NULL);
		if (SOCKET_ERROR == ret) {
			int error_num = WSAGetLastError();
			if (ERROR_IO_PENDING != error_num)
				error_display(error_num);
		}
	}

};



struct timer_ev {

	int this_id;
	int target_id;
	chrono::system_clock::time_point   start_t;
	EVENT_TYPE order;
	constexpr bool operator < (const timer_ev& _Left) const
	{
		return (start_t > _Left.start_t);
	}
};

class EscapeObject
{
public:
	float x, y, z;
	float pitch, yaw, roll;
	int ob_id;
	int currentGameRoom;
public:
	EscapeObject() {}
	~EscapeObject() {}

	void setPosition(float x_val, float y_val, float z_val) {
		x = x_val;
		y = y_val;
		z = z_val;
	}

	void removeOBJ() {
		//삭제되었을 때 배열을 밀지말고 돌려야함 삭제된 아이템 정보를 상대들에게 동기화
	}
};
template<typename T>
class LockQueue
{
public:
	LockQueue() { }

	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(_mutex);
		_queue.push(std::move(value));
		_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_queue.empty())
			return false;

		value = std::move(_queue.front());
		_queue.pop();
		return true;
	}

	void WaitPop(T& value)
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] { return _queue.empty() == false; });
		value = std::move(_queue.front());
		_queue.pop();
	}

	void Clear()
	{
		unique_lock<mutex> lock(_mutex);
		if (_queue.empty() == false)
		{
			queue<T> _empty;
			swap(_queue, _empty);
		}
	}
	bool Empty()
	{
		lock_guard<mutex> lock(_mutex);
		return _queue.empty();
	}
private:
	queue<T> _queue;
	mutex _mutex;
	condition_variable _condVar;
};


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
void send_put_object(int _s_id, int target);
void Disconnect(int _s_id);
void send_ready_packet(int _s_id);
void send_travel_ready_packet(int _s_id);
void send_endgame_packet(int _s_id);
void send_myitem_count_packet(int _s_id);
void send_item_packet(int _s_id, int item_index);
void send_myitem_packet(int _s_id);
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


//해제
void Disconnect(int _s_id)
{
	CLIENT& cl = clients[_s_id];
	clients[_s_id].state_lock.lock();
	clients[_s_id]._state = ST_FREE;
	clients[_s_id].state_lock.unlock();
	closesocket(clients[_s_id]._socket);
	cout << "------------연결 종료------------" << endl;
}


//패킷 판별
void process_packet(int s_id, unsigned char* p)
{
	unsigned char packet_type = p[1];
	//CLIENT& cl = clients[s_id];
	//cout << "user : " << s_id << "packet type :" << to_string(packet_type) << endl;
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
		//cout << "cl.sid?= " << packet->id << ", 00 , " << cl._s_id << endl;
		cl.x = packet->x;
		cl.y = packet->y;
		cl.z = packet->z;
		cl.p_type = packet->p_type;
		cl.connected = true;
		ingamecount++;
		send_select_character_type_packet(cl._s_id);

		//cout << "몇명 들어옴 : " << ingamecount << endl;

		if (ingamecount >= 2)
		{
			for (auto& player : clients) {
				if (ST_INGAME != player._state)
					continue;

				send_ready_packet(player._s_id);
				//cout << "보낼 플레이어" << player._s_id << endl;

			}
		}
		break;
	}
	case CS_MOVE_Packet: {
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
		cl.AO_PITCH = packet->AO_pitch;
		cl.AO_YAW = packet->AO_yaw;
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
		//cout << "플레이어 : " << cl._s_id << "무기 타입" << cl.w_type << endl;
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
		//	cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_READY: {
		CS_READY_PACKET* packet = reinterpret_cast<CS_READY_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		//cout << "Ready id" << packet->id;
		ready_count++;
		//cout << "ready_count" << ready_count << endl;
		if (ready_count >= 2)
		{
			for (auto& player : clients) {
				if (ST_INGAME != player._state)
					continue;
				send_travel_ready_packet(player._s_id);
			//	cout << "보낼 플레이어" << player._s_id << endl;
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

		//	cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);

		}
		break;
	}
	case CS_START_GAME: {
		CS_START_GAME_PACKET* packet = reinterpret_cast<CS_START_GAME_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
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
			packet.Max_speed = cl.Max_Speed;
			packet.p_type = cl.p_type;
			//printf_s("[Send put object] id : %d, location : (%f,%f,%f), yaw : %f\n", packet.id, packet.x, packet.y, packet.z, packet.yaw);
			//cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
			other.do_send(sizeof(packet), &packet);
		}
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
		//cout << "weptype : " << cl.wtype << endl;
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
	case CS_BOJOWEAPON: {
		CS_BOJOWEAPON_PACKET* packet = reinterpret_cast<CS_BOJOWEAPON_PACKET*>(p);
		CLIENT& cl = clients[packet->attack_id];
		cl.s_x = packet->lx;
		cl.s_y = packet->ly;
		cl.s_z = packet->lz;
		cl.Pitch = packet->r_pitch;
		cl.Yaw = packet->r_yaw;
		cl.Roll = packet->r_roll;
		cl.wtype = packet->wep_type;
		cout << "id : " << packet->attack_id << "weptype : " << cl.wtype << endl;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_BOJOWEAPON_PACKET packet;
			packet.attack_id = cl._s_id;
			packet.size = sizeof(packet);
			packet.type = SC_BOJOWEAPON;
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
			//cout << "이거 누구한테 감 :  ?" << other._s_id << endl;
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
		//cout << "aaaaa들어옴?" << endl;
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

	//	cout << "누가 이김 " << packet->id << endl;
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
		cout << "이거 언제 들어옴?" << endl;
		CLIENT& cl = clients[packet->id];
		cl.myItemCount = packet->itemCount;
		//send_myitem_packet(cl._s_id);

	//	cout << "내가 획득한 아이템 개수" << cl._s_id << " : " << cl.myItemCount << endl;
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
	case CS_ALIVE: {
		CS_ALIVE_PACKET* packet = reinterpret_cast<CS_ALIVE_PACKET*>(p);
	//	cout << "packet->id : " << packet->id << endl;
		CLIENT& cl = clients[packet->id];
		cl.deadtype = packet->deadtype;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_ALIVE_PACKET packet;


			packet.size = sizeof(packet);
			packet.type = SC_ALIVE;
			packet.id = cl._s_id;
			packet.deadtype = cl.deadtype;
			other.do_send(sizeof(packet), &packet);
		}

		break;
	}
	case CS_DISSOLVE: {
		CS_DISSOLVE_PACKET* packet = reinterpret_cast<CS_DISSOLVE_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		if (packet->dissolve == 1) {
			cout << "dissolve : " << packet->dissolve << endl;
			cout << "id : " << packet->id << endl;
		}
		cl.dissolve = packet->dissolve;
		for (auto& other : clients) {
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_DISSOLVE_PACKET packet;

			packet.size = sizeof(packet);
			packet.type = SC_DISSOLVE;
			packet.id = cl._s_id;
			packet.dissolve = cl.dissolve;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_REMOVE_ITEM: {
		CS_REMOVE_ITEM_PACKET* packet = reinterpret_cast<CS_REMOVE_ITEM_PACKET*>(p);
		CLIENT& cl = clients[s_id];
		//cl.myItemCount += 1;
		//cout << "itemid : " << packet->itemid << endl;
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
			packet.id = cl._s_id;
			//packet.itemcount = cl.myItemCount;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_INCREASE_COUNT: {
		CS_INCREASE_ITEM_PACKET* packet = reinterpret_cast<CS_INCREASE_ITEM_PACKET*>(p);
		CLIENT& cl = clients[packet->Increaseid];
		cl.myItemCount = packet->itemCount;
		cout << "packet->id : " << packet->Increaseid << "packet->itemcount" << packet->itemCount << endl;
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
		CS_DECREASE_ITEM_PACKET* packet = reinterpret_cast<CS_DECREASE_ITEM_PACKET*>(p);
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
			CS_DECREASE_ITEM_PACKET packet;
			packet.Increaseid = cl._s_id;
			strcpy_s(packet.cid, cl.name);
			packet.itemCount = cl.myItemCount;
			packet.size = sizeof(packet);
			packet.type = SC_DECREASE;
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
	case CS_DAMAGE: {
		CS_DAMAGE_PACKET* packet = reinterpret_cast<CS_DAMAGE_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cl._hp = packet->hp;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			SC_HP_CHANGE_PACKET packet;
			packet.size = sizeof(packet);
			packet.type = SC_HP_CHANGE;
			packet.id = cl._s_id;
			packet.HP = cl._hp;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_BOJO_ANIM: {
		CS_BOJO_ANIM_PACKET* packet = reinterpret_cast<CS_BOJO_ANIM_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cl.bojoanimtype = packet->bojoanimtype;
		for (auto& other : clients) {
			if (other._s_id == cl._s_id) continue;
			other.state_lock.lock();
			if (ST_INGAME != other._state) {
				other.state_lock.unlock();
				continue;
			}
			else other.state_lock.unlock();
			CS_BOJO_ANIM_PACKET packet;
			packet.size = sizeof(packet);
			packet.type = SC_BOJO_ANIM;
			packet.id = cl._s_id;
			packet.bojoanimtype = cl.bojoanimtype;
			other.do_send(sizeof(packet), &packet);
		}
		break;
	}
	case CS_MOPP: {
		CS_MOPP_PACKET* packet = reinterpret_cast<CS_MOPP_PACKET*>(p);
		CLIENT& cl = clients[packet->id];
		cout << "itemid : " << packet->itemid << endl;
		int itemid = packet->itemid;
		int mopptype = packet->mopptype;
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
			unsigned char* packet_start = exp_over->_net_buf;
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
			if (remain_data == 0)
				cl._prev_size = 0;
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
	packet.AO_pitch = clients[target].AO_PITCH;
	packet.AO_yaw = clients[target].AO_YAW;
	clients[_id].do_send(sizeof(packet), &packet);
}

void send_change_hp(int _s_id)
{
	SC_HP_CHANGE_PACKET packet;
	packet.size = sizeof(packet);
	packet.id = _s_id;
	packet.type = SC_HP_CHANGE;
	packet.HP = clients[_s_id]._hp;
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

	cout << "_s_id, packetsize" << _s_id << sizeof(packet) << endl;
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