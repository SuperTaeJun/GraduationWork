#pragma once
#define MAX_INFO_SIZE   20

const char CS_LOGIN = 1;
const char SC_LOGIN_OK = 1;

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
#pragma pack(pop)
