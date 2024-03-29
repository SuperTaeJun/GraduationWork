// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//---------------------------------
// 패킷 프로토콜 클래스
//---------------------------------


#define	MAX_BUFFER		256
#define SERVER_PORT		8001
#define SERVER_IP		"192.168.219.101"
#define MAX_CLIENTS		100
#define MAX_INFO_SIZE   20

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
const char CS_LOGIN = 1;
const char CS_MOVE = 2;
const char CS_SELECT_CHAR = 3;
const char CS_SELECT_WEP = 4;
const char SC_LOGIN_OK = 1;
const char SC_OTHER_PLAYER = 2;
const char SC_MOVE_PLAYER = 3;
const char SC_CHAR_BACK = 4;
const char SC_OTHER_WEAPO = 5;

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
	int32 clientid;
	/*float x, y, z;
	float yaw;
	int32 cl_id;*/
	//PlayerType p_type;
};
#pragma pack(pop)

#pragma pack (push, 1)
struct CS_MOVE_PACKET
{
	unsigned char size;
	char type;
	int	id;
	float Max_speed;
	float x, y, z;
	float vx, vy, vz;
	float yaw;
};

struct SC_PLAYER_SYNC {
	unsigned char size;
	char type;
	int id;
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
	int id;
	//PlayerType character_type;
};
struct CS_SELECT_WEAPO
{
	unsigned char size;
	char type;
	int id;
	WeaponType weapon_type;
};

struct SC_SYNC_WEAPO
{
	unsigned char size;
	char type;
	int id;
	WeaponType weapon_type;
};
#pragma pack(pop)

/**
 * 
 */
class BREAKOUT_API PacketData
{
public:
};
