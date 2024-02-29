// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//---------------------------------
// 패킷 프로토콜 클래스
//---------------------------------


#define	MAX_BUFFER		4096
#define SERVER_PORT		12345
#define SERVER_IP		"127.0.0.1"
#define MAX_CLIENTS		100
#define MAX_INFO_SIZE   20

const char CS_LOGIN = 1;
const char CS_MOVE = 2;


const char SC_LOGIN_OK = 1;
//const char SC_MOVE_OK = 2;
const char SC_OTHER_PLAYER = 3;

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
	float x, y, z;
	float yaw;
	char object_type;
	char name[MAX_INFO_SIZE];
};

#pragma pack(pop)


/**
 * 
 */
class BREAKOUT_API PacketData
{
public:
};
