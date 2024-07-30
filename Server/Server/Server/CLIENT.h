#pragma once
#include "pch.h"

class CLIENT
{
public:
	int _s_id; //플레이어 고유 id
	char name[MAX_NAME_SIZE]; //플레이어 nick
	char _pw[MAX_NAME_SIZE];  // pw
	//로그인 상태
	bool bLogin = false;
	// 위치
	float	x;
	float	y;
	float	z;
	// 회전값
	// 아이템 획득 카운트
	int myItemCount;
	bool connected = false;
	bool selectweapon = false;
	float	Yaw;
	float	Pitch;
	float	Roll;
	float   Speed;
	// 에임 오프셋
	float AO_YAW, AO_PITCH;
	// 속도
	float VX;
	float VY;
	float VZ;
	float Max_Speed;

	float _hp; // 체력
	int damage;
	bool is_bone = false;
	bool bGetWeapon = false;
	bool bCancel;
	bool bEndGame = false;
	// 충전
	bool bRecharge = false;
	// bAlive
	bool bAlive = true;
	// 라이트 온/오프
	bool bLightOn = false;
	//	Reload
	bool bReload = false;
	// hitAnim
	bool bHitAnim = false;
	WeaponType w_type;
	PlayerType p_type;
	float s_x, s_y, s_z;
	float e_x, e_y, e_z;
	int wtype;
	//보조무기 애니메이션 상태
	int bojoanimtype;
	//--------------------
	//죽는 애니메이션 타입
	int deadtype;
	// 디졸브 타입
	int dissolve;
	// 입장한 게임룸 번호
	int currentRoom = -1;
	int jumpType = -1;
	unordered_set   <int>  viewlist; // 시야 안 오브젝트

	mutex state_lock;
	CL_STATE _state;
	atomic_bool  _is_active = false;
	int num;
	int itemAnimNum;
	//-------------
	float pitch0, yaw0, roll0;
	float pitch1, yaw1, roll1;
	float pitch2, yaw2, roll2;
	float pitch3, yaw3, roll3;
	float pitch4, yaw4, roll4;
	//-------------
	Overlap		_recv_over;
	SOCKET		_socket;
	int      _prev_size;
	int      last_move_time;

public:
	CLIENT() : _state(ST_FREE), _prev_size(0)
	{
		_hp = 100.f;
		myItemCount = 0;
	}

	~CLIENT()
	{
		closesocket(_socket);
	}
	void do_recv();
	void do_send(int num_bytes, void* mess);
};


