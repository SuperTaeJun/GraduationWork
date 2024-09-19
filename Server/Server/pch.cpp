#include "pch.h"

array <CLIENT, MAX_USER> clients;
atomic<int> ready_count;
atomic<int> ingamecount; 
array<EscapeObject, 8> objects;
array<BulletWall, 9> walls;
RoomManager roomManager;


void error_display(int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, 0);
	wcout << lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
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


//로그인 허용
void send_login_ok_packet(int _s_id)
{
	SC_LOGIN_BACK packet;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_OK;
	packet.id = _s_id;
	packet.bLogin = clients[_s_id].bLogin;
	strcpy_s(packet.cid, clients[_s_id].name);
	cout << "_s_id" << _s_id << endl;

	clients[_s_id].do_send(sizeof(packet), &packet);
}
void send_login_fail_packet(int _s_id, int failType)
{
	SC_LOGIN_FAIL_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_LOGIN_FAIL;
	packet.id = _s_id;
	packet.failType = failType;
	clients[_s_id].do_send(sizeof(packet), &packet);
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
	packet.jumpType = clients[target].jumpType;
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
void send_bullet_wall(int _s_id, int wall_index)
{
	CS_WALL_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_WALL;

	packet.lx = walls[wall_index].x;
	packet.ly = walls[wall_index].y;
	packet.lz = walls[wall_index].z;
	packet.r_pitch = walls[wall_index].pitch;
	packet.r_yaw = walls[wall_index].yaw;
	packet.r_roll = walls[wall_index].roll;
	packet.wall_id = walls[wall_index].ob_id;

	clients[_s_id].do_send(sizeof(packet), &packet);
}

void SendLobbyPacket(int clientId, bool bintoRoom)
{
	SC_LOBBY_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = SC_LOBBY_ROOM;
	packet.id = clientId;
	packet.bLobby = bintoRoom;
	clients[clientId].do_send(sizeof(packet), &packet);
}