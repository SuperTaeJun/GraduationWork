#pragma once
#define SERVER_PORT		8001
#define SERVER_IP		"192.168.87.28"

#define MAX_INFO_SIZE   20
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
	/*float x, y;
	float z;*/
	//PlayerType p_type;

};	
struct SC_LOGIN_BACK {
	unsigned char size;
	char type;
	/*char id[MAX_INFO_SIZE];
	char pw[MAX_INFO_SIZE];*/
	int32 clientid;
	/*float x, y, z;
	float yaw;
	int32 cl_id;*/
	//PlayerType p_type;
};

struct CS_MOVE_PACKET
{
	unsigned char size;
	char type;
	int32	id;
	float Max_speed;
	float x, y, z;
	float vx, vy, vz;
	float yaw;
};
struct SC_PLAYER_SYNC {
	unsigned char size;
	char type;
	int32 id;
	float Max_speed;
	float x, y, z;
	float yaw;
	//char object_type;
	char name[MAX_INFO_SIZE];
	//PlayerType p_type;
};
struct CS_SELECT_CHARACTER
{
	unsigned char size;
	char type;
	int32 id;
	//PlayerType character_type;
};
struct CS_SELECT_WEAPO
{
	unsigned char size;
	char type;
	int32 id;
	WeaponType weapon_type;
};
struct SC_SYNC_WEAPO
{
	unsigned char size;
	char type;
	int32 id;
	WeaponType weapon_type;
};
#pragma pack(pop)








//
//struct cs_packet_start { // 게임 레디 요청
//	unsigned char size;
//	char	type;
//	bool	ready;
//};
//struct sc_packet_ready { // 타 플레이어 레디
//	unsigned char size;
//	char	type;
//	char	name[MAX_NAME_SIZE];
//};
//
//struct sc_packet_start_ok { // 스폰
//	unsigned char size;
//	char type;
//	char	name[MAX_NAME_SIZE];
//	float x, y, z;
//	char image_num;
//};
//
//struct cs_packet_attack {
//	unsigned char size;
//	char	type;
//	int s_id;
//};
//
//struct cs_packet_damage {
//	unsigned char size;
//	char	type;
//};
//
//struct cs_packet_get_item {
//	unsigned char size;
//	char	type;
//	int s_id;
//	char    item_num;
//};
//
//struct cs_packet_chat {
//	unsigned char size;
//	char	type;
//	int s_id;
//	float x, y, z;
//	char	message[MAX_CHAT_SIZE];
//};
//
//struct cs_packet_teleport {
//	// 서버에서 장애물이 없는 랜덤 좌표로 텔레포트 시킨다.
//	// 더미 클라이언트에서 동접 테스트용으로 사용.
//	unsigned char size;
//	char	type;
//};
//
//struct sc_packet_remove_object {
//	unsigned char size;
//	char type;
//	int id;
//};
//
//struct sc_packet_chat {
//	unsigned char size;
//	char type;
//	int id;
//	char message[MAX_CHAT_SIZE];
//};
//
//struct sc_packet_login_fail {
//	unsigned char size;
//	char type;
//	int	 reason;		// 0: 중복 ID,  1:사용자 Full
//};
//
//struct sc_packet_status_change {
//	unsigned char size;
//	char type;
//	short   state;
//	short	hp, maxhp;
//	bool ice[4]; // 사지분해
//};
//
//
//struct sc_packet_hp_change {
//	unsigned char size;
//	char type;
//	int target;
//	int	hp;
//};