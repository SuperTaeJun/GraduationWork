#pragma once
#define SERVER_PORT		8001
#define SERVER_IP		"192.168.213.28"
#define MAX_INFO_SIZE   20
#include <vector>
#include <array>

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

constexpr int BUFSIZE = 1048;
const int  MAX_NAME_SIZE = 20;
const int  MAX_USER = 10000;
const int  MAX_OBJ = 11;


const char CS_MOVE_Packet = 0;
const char CS_LOGIN = 1;
const char CS_SELECT_CHAR = 3;
const char CS_SELECT_WEP = 4;
const char CS_READY = 5;
const char CS_HIT_EFFECT = 7;
const char CS_SIGNAl = 9;
const char CS_SHOTGUN_BEAM = 10;
const char CS_NiAGARA = 12;
const char CS_NiAGARA_CANCEL = 13;
const char CS_START_GAME = 14;
const char CS_NiAGARA_CH1 = 15;
const char CS_END_GAME = 16;
const char CS_GETITEM = 17;
const char CS_STOP_ANIM = 18;
const char CS_REMOVE_ITEM = 19;
const char CS_INCREASE_COUNT = 20;
const char CS_ITEM_INFO = 21;
const char CS_RELOAD = 22;
const char CS_ITEM_ANIM = 23;
const char CS_REMOVE_WEAPON = 24;
const char CS_DECREASE_COUNT = 25;
const char CS_CH2_SKILL = 26;
const char CS_PLAYER_HEAL = 27;
const char CS_PLAYER_RELOAD = 28;
const char CS_MOPP = 29;

const char SC_LOGIN_OK = 1;
const char SC_OTHER_PLAYER = 2;
const char SC_MOVE_PLAYER = 3;
const char SC_CHAR_BACK = 4;
const char SC_OTHER_WEAPO = 5;
const char SC_ALL_READY = 6;
const char SC_ATTACK = 7;
const char SC_EFFECT = 8;
const char SC_PLAYER_DAMAGE = 9;
const char SC_SHOTGUN_BEAM = 10;
const char SC_SHOTGUN_DAMAGED = 11;
const char SC_NiAGARA = 12;
const char SC_NiAGARA_CANCEL = 13;
const char SC_SIGNAL = 14;
const char SC_NiAGARA_CH1 = 15;
const char SC_ITEM = 16;
const char SC_END_GAME = 17;
const char SC_ITEM_ACQUIRE = 18;
const char SC_STOP_ANIM = 19;
const char SC_REMOVE_ITEM = 20;
const char SC_MYITEM_COUNT = 21;
const char SC_INCREASE_COUNT = 22;
const char SC_RELOAD = 23;
const char SC_ITEM_ANIM = 24;
const char SC_REMOVE_WEAPON = 25;
const char SC_MYNEW_COUNT = 26;
const char SC_CH2_SKILL = 27;
const char SC_PLAYER_HEAL = 28;
const char SC_PLAYER_RELOAD = 29;
const char SC_MOPP = 30;
#pragma pack (push, 1)
struct CS_LOGIN_PACKET
{
	unsigned char size;
	unsigned type;
	char id[MAX_INFO_SIZE];
	char pw[MAX_INFO_SIZE];
};
struct SC_LOGIN_BACK {
	unsigned char size;
	unsigned type;
	int id;
	char cid[MAX_INFO_SIZE];

};
#pragma pack(pop)


#pragma pack (push, 1)
struct CS_MOVE_PACKET
{
	unsigned char size;
	unsigned type;
	int	id;
	float Max_speed;
	float x, y, z;
	float vx, vy, vz;
	float yaw;
};
struct SC_PLAYER_SYNC {
	unsigned char size;
	unsigned type;
	int id;
	float Max_speed;
	float x, y, z;
	float yaw;
	//char object_type;
	char name[MAX_INFO_SIZE];
	PlayerType p_type;
};
struct CS_SELECT_CHARACTER
{
	unsigned char size;
	unsigned type;
	int id;
	float x, y, z;
	PlayerType p_type;
};
struct SC_SELECT_CHARACTER_BACK {
	unsigned char size;
	unsigned type;
	int clientid;
	float x, y, z;
	PlayerType p_type;
};
struct CS_SELECT_WEAPO
{
	unsigned char size;
	unsigned type;
	int id;
	bool bselectwep;
	WeaponType weapon_type;
};
struct SC_SYNC_WEAPO
{
	unsigned char size;
	unsigned type;
	int id;
	bool bselectwep;
	WeaponType weapon_type;
};
struct CS_READY_PACKET {
	unsigned char size;
	unsigned type;
};
struct SC_ACCEPT_READY {
	unsigned char size;
	unsigned type;
	bool ingame;
};
struct CS_ATTACK_PLAYER {
	unsigned char size;
	unsigned char type;
	int attack_id;
	float sx, sy, sz;
	float ex, ey, ez;
};
struct SC_ATTACK_PLAYER {
	unsigned char size;
	unsigned char type;
	int clientid;
	float sx, sy, sz;
	float ex, ey, ez;
};
struct CS_EFFECT_PACKET {
	unsigned char size;
	unsigned char type;
	int attack_id;
	float lx, ly, lz;
	float r_pitch, r_yaw, r_roll;
	// 0 = 라이플, 1, 런처
	int wep_type;
};
struct CS_DAMAGE_PACKET {
	unsigned char size;
	unsigned char type;
	int damaged_id;
	float damage;
};
struct SC_DAMAGE_CHANGE
{
	unsigned char size;
	unsigned char type;
	int damaged_id;
	int damage;
};
struct CS_SIGNAL_PACKET
{
	unsigned char size;
	unsigned char type;
	int id;
	int num;
};
#pragma pack(pop)
#pragma pack (push, 1)
struct CS_SHOTGUN_BEAM_PACKET {
	unsigned char size;
	unsigned char type;
	int attackid;
	float sx, sy, sz;
	float pitch0, yaw0, roll0;
	float pitch1, yaw1, roll1;
	float pitch2, yaw2, roll2;
	float pitch3, yaw3, roll3;
	float pitch4, yaw4, roll4;
	float pitch5, yaw5, roll5;
	float pitch6, yaw6, roll6;
	float pitch7, yaw7, roll7;
	float pitch8, yaw8, roll8;

	//float ex9, ey9, ez9;
};
struct CS_SHOTGUN_DAMAGED_PACKET {
	unsigned char size;
	unsigned char type;
	int damaged_id;
	int damaged_id1;
	int damaged_id2;
	float damage;
	float damage1;
	float damage2;
};
struct SC_SHOTGUN_DAMAGED_CHANGE_PACKET {
	unsigned char size;
	unsigned char type;
	int damaged_id1;
	int damaged_id2;
	int damaged_id3;
	float newhp1;
	float newhp2;
	float newhp3;
};
struct CS_NIAGARA_SYNC_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	PlayerType playertype;
	int num;
};
struct CS_NIAGARA_CANCEL_PACKET {
	unsigned char size;
	unsigned char type;
	bool cancel;
	int id;
	int num;
};
struct CS_NIAGARA_PACKETCH1 {
	unsigned char size;
	unsigned char type;
	int id;
	PlayerType playertype;
	float x, y, z;
	int num;
};

struct CS_START_GAME_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
};
struct SC_ITEM_PACKET {
	unsigned char size;
	unsigned char type;
	float x, y, z;
	float pitch, yaw, roll;
	int id;
};
struct CS_END_GAME_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	int winnerid;
	bool bEND;
};
struct CS_ITEM_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	int itemCount;
};
struct SC_ITEM_ACQUIRE_PACKET {
	unsigned char size;
	unsigned char type;
	int acquireid;
	int id;
	char cid[MAX_INFO_SIZE];
	int itemCount;
};
struct CS_STOP_ANIM_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	bool bStopAnim;
};
struct CS_REMOVE_ITEM_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	int itemid;
};
struct SC_MY_ITEM_COUNT {
	unsigned char size;
	unsigned char type;
	int id;
	int MyITEMCount;
};
struct CS_INCREASE_ITEM_PACKET {
	unsigned char size;
	unsigned char type;
	char cid[MAX_INFO_SIZE];
	int Increaseid;
	int itemCount;
};
struct CS_ITEM_INFO_PACKET {
	unsigned char size;
	unsigned char type;
	int objid;
};
struct CS_RELOAD_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	bool bReload;
};
struct CS_ITEM_ANIM_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	int num;
};
struct CS_REMOVE_WEAPON_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	bool bWeapon;
};
struct SC_MYNEW_ITEM_COUNT {
	unsigned char size;
	unsigned char type;
	int id;
	int MyITEMCount;
};
struct SC_CH2_SKILL_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	PlayerType p_type;
	bool bfinish;
};
struct CS_HEAL_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	float hp;
};
struct CS_RELOAD_TIMER_PACKET {
	unsigned char size;
	unsigned char type;
	int id;
	int bulletcount;
};
struct CS_MOPP_PACKET {
	unsigned char size;
	unsigned char type;
	int itemid;
	int mopptype;
	float DeltaTime;
};
#pragma pack(pop)
