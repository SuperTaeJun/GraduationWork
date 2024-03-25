#pragma once

#define MAX_INFO_SIZE   20
const short SERVER_PORT = 7777;

const int BUFSIZE = 256;
const int  ReZone_HEIGHT = 2000;
const int  ReZone_WIDTH = 2000;
const int  MAX_NAME_SIZE = 20;
const int  MAX_CHAT_SIZE = 100;
const int  MAX_USER = 10000;
const int  MAX_OBJ = 20;

const char CS_LOGIN = 1;
const char CS_MOVE = 2;
const char CS_SELECT_CHAR = 3;
const char CS_SELECT_WEP = 4;
const char SC_LOGIN_OK = 1;
const char SC_OTHER_PLAYER = 2;
const char SC_MOVE_PLAYER = 3;
const char SC_CHAR_BACK = 4;
const char SC_OTHER_WEAPO = 5;

//const char CS_PACKET_ATTACK = 3;
enum PlayerType
{
	Character1,
	Character2,
	Character3,
	Character4
};

enum WeaponType
{
	RIFLE,
	SHOTGUN,
	LAUNCHER
};
//const char CS_PACKET_DAMAGE = 7;
//const char CS_PACKET_GET_ITEM = 8;
//
//const char SC_LOGIN_OK = 1;
//const char SC_OTHER_PLAYER = 2;
//const char SC_MOVE_PLAYER = 3;
//
//const char SC_PACKET_REMOVE_OBJECT = 4;
//
//const char SC_PACKET_LOGIN_FAIL = 6;
//const char SC_PACKET_STATUS_CHANGE = 7;
//const char SC_PACKET_DISCONNECT = 8;
//const char SC_PACKET_HP = 9;
//
//const char SC_PACKET_ATTACK = 11;
//const char SC_PACKET_GET_ITEM = 12;

#pragma pack (push, 1)
struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
	char id[MAX_INFO_SIZE];
	char pw[MAX_INFO_SIZE];
	float x, y;
	float z;
	PlayerType p_type;

};
struct SC_LOGIN_BACK {
	unsigned char size;
	char type;
	/*char id[MAX_INFO_SIZE];
	char pw[MAX_INFO_SIZE];*/
	int clientid;
	float x, y, z;
	float yaw;
	//int cl_id;
	PlayerType p_type;
};
struct CS_MOVE_PACKET
{
	unsigned char size;
	char type;
	int	clientid;
	float Max_speed;
	float x, y, z;
	float vx, vy, vz;
	float yaw;
};
//동기화 용 패킷
struct SC_PLAYER_SYNC {
	unsigned char size;
	char type;
	int clientid;
	float Max_speed;
	float x, y, z;
	float yaw;
	char object_type;
	char name[MAX_INFO_SIZE];
	PlayerType p_type;
};
struct CS_SELECT_CHARACTER
{
	unsigned char size;
	char type;
	int clientid;
	//PlayerType character_type;
};
struct CS_SELECT_WEAPO
{
	unsigned char size;
	char type;
	int clientid;
	WeaponType weapon_type;
};
struct SC_SYNC_WEAPO
{
	unsigned char size;
	char type;
	int clientid;
	WeaponType weapon_type;
};
#pragma pack(pop)
