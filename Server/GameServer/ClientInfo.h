#pragma once
#include "CorePch.h"
#include "Overlapped.h"

//로그인 정보 struct
enum Character{
	Character_ARION,
	Character_DANTE,
	Character_EDAN,
	Character_KRATOS
};
enum Weapon {
	Weapon_Rifle,
	Weapon_Shotgun,
	Weapon_RocketLauncher,
};

struct LoginInfo {
	 char id[20];
	 char pw[20];
	 Character c_type;
	 Weapon w_type;
	 int Kill_num;
	 int Death_num;
};
enum CL_STATE { ST_FREE, ST_ACCEPT, ST_INGAME, ST_SERVER }; //  접속 상태
// 클라들 정보 저장 클래스 
class ClientInfo
{
public:
	int cl_id;
	LoginInfo LoginInfo;
	float x, y, z;
	float Yaw, Pitch, Roll;
	float VX, VY, VZ;
	int hp;
	SOCKET c_socket;
	Overlapped c_overlapped;
	int prev;
	CL_STATE cl_state;  //  접속 상태
public:
	ClientInfo();
	~ClientInfo() { closesocket(c_socket); }
	bool c_recv();
	bool c_send(int num_bytes, void* mess);
private:

};

