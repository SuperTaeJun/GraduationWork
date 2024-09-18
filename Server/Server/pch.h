#pragma once
#include <iostream>
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
#include <windows.h>
#include <sqlext.h>
using namespace std;

#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")

#include "Enum.h"
#include "protocol.h"
#include "Overlap.h"
#include "DB.h"
#include "CLIENT.h"
#include "LockQueue.h"
#include "Room.h"
#include "PacketManager.h"
#include "BulletWall.h"
#include "EscapeObject.h"

struct timer_ev {

	int this_id{};
	int target_id{};
	chrono::system_clock::time_point   start_t;
	EVENT_TYPE order;
	constexpr bool operator < (const timer_ev& _Left) const
	{
		return (start_t > _Left.start_t);
	}
};


void error_display(int err_no);

extern array <CLIENT, MAX_USER> clients;
extern atomic<int> ready_count;
extern atomic<int> ingamecount;
extern array<EscapeObject, 8> objects;
extern array<BulletWall, 9> walls;

void send_select_character_type_packet(int _s_id);
void send_login_ok_packet(int _s_id);
void send_login_fail_packet(int _s_id, int failType);
void send_move_packet(int _id, int target);
void send_change_hp(int _s_id);
void send_ready_packet(int _s_id);
void send_travel_ready_packet(int _s_id);
void send_endgame_packet(int _s_id);
void send_myitem_packet(int _s_id);
void send_myitem_count_packet(int _s_id);
void send_item_packet(int _s_id, int item_index);
void send_bullet_wall(int _s_id, int wall_index);
void SendLobbyPacket(int clientId, bool bintoRoom);