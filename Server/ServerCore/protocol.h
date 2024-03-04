#pragma once
#define MAX_INFO_SIZE   20

const char CS_LOGIN = 1;
const char CS_MOVE = 2;


const char SC_LOGIN_OK = 1;
const char SC_OTHER_PLAYER = 2;
const char SC_OWN_MOVE = 3;

#pragma pack (push, 1)
struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
	char id[MAX_INFO_SIZE];
	char pw[MAX_INFO_SIZE];

};
struct SC_LOGIN_BACK {
	unsigned char size;
	char type;
	char id[MAX_INFO_SIZE];
	char pw[MAX_INFO_SIZE];
	float x, y, z;
	int cl_id;
};
struct CS_MOVE_PACKET
{
	unsigned char size;
	char type;
	int	id;
	float x, y, z;
	float vx, vy, vz;
	float yaw;
};
//struct SC_MOVE_BACK {
//	unsigned char size;
//	char type;
//	int		id;
//	float x, y, z;
//};
//동기화 용 패킷
struct SC_PLAYER_SYNC {
	unsigned char size;
	char type;
	int id;
	float x, y,z;
	float yaw;
	char object_type;
	char name[MAX_INFO_SIZE];
};

#pragma pack(pop)
