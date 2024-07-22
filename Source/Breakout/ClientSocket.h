// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <queue>
#include <fstream>
#include <map>
#include <mutex>
#include "../../Server/Server/protocol.h"
#include <vector>
#include <iostream>
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "CoreMinimal.h"

class UBOGameInstance;
class ACharacterController;
using namespace std;


template<typename T>
class LockQueue
{
public:
	LockQueue() { }

	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

	void Push(T value)
		//void Push(int32 value)
	{
		lock_guard<mutex> lock(_mutex);
		_queue.push(std::move(value));
		_condVar.notify_one();
	}

	bool TryPop(T& value)
		//bool TryPop(int32& value)
	{
		lock_guard<mutex> lock(_mutex);
		if (_queue.empty())
			return false;

		value = std::move(_queue.front());
		_queue.pop();
		return true;
	}

	void WaitPop(T& value)
		//void WaitPop(int32& value)
	{
		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [this] { return _queue.empty() == false; });
		value = std::move(_queue.front());
		_queue.pop();
	}

	void Clear()
	{
		unique_lock<mutex> lock(_mutex);
		if (_queue.empty() == false)
		{
			queue<T> _empty;
			//queue<int32> _empty;
			swap(_queue, _empty);
		}
	}

	int Size()
	{
		unique_lock<mutex> lock(_mutex);
		return _queue.size();
	}


private:
	std::queue<T> _queue;
	//std::queue<int32> _queue;
	std::mutex _mutex;
	std::condition_variable _condVar;
};


const int buffsize = 2048;


enum IO_type
{
	IO_RECV,
	IO_SEND,
	IO_ACCEPT,
};

class Overlap {
public:
	WSAOVERLAPPED   _wsa_over;
	IO_type         _op;
	WSABUF         _wsa_buf;
	char   _net_buf[buffsize];
	int            _target;
public:
	Overlap(IO_type _op, char num_bytes, void* mess) : _op(_op)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
		_wsa_buf.len = num_bytes;
		memcpy(_net_buf, mess, num_bytes);
	}

	Overlap(IO_type _op) : _op(_op) {}

	Overlap()
	{
		_op = IO_RECV;
	}

	~Overlap()
	{
	}
};

// 플레이어 클래스 
class CPlayer
{
public:
	CPlayer() { };
	~CPlayer() {};

	// 세션 아이디
	int Id = -1;
	int WinnerID = -1;
	//int hp;
	//주무기 타입
	int weptype;
	// 보조무기 타입
	int bojotype = 3;
	float hp = 100.f;
	int skilltype;
	int itemAnimtype = -1;
	// 죽는 애니메이션 타입
	int deadtype = 2;
	// 디졸브 상태
	int	dissolve = 2;
	// 보조무기 애니메이션
	int bojoanimtype = 2;
	// 아이템 개수
	int itemCount = 0;
	// 아이디 비번
	FString    userId = {};
	FString    userPw = {};
	// 위치
	float X;
	float Y = 0;
	float Z = 0;
	// 회전값
	float Yaw = 0;
	float Pitch = 0;
	float Roll = 0;
	// 속도
	float VeloX = 0;
	float VeloY = 0;
	float VeloZ = 0;
	float Max_Speed = 400;
	// 에임오프셋
	float AO_YAW = 0;
	float AO_PITCH = 0;
	//
	bool  bAlive = true;
	bool  bGetWeapon = false;
	bool  IsAlive = true;
	bool  fired = false;
	bool  sfired = false;
	bool  hiteffect = false;
	bool  brecvdamage = false;
	bool  bselectweapon = false;
	bool  bniagara = false;
	bool  bniagarach1 = false;
	bool  bch4end = false;
	bool  bEndGame = false;
	bool  bServerReload = false;
	bool  bFinishSkill = false;
	// 나이아가라 슛 이팩트
	FVector Sshot;
	FVector Eshot;
	// 나이아가라 히팅 이팩트
	FVector Hshot;
	//샷건 슛 이팩트
	FVector sSshot;
	FRotator sEshot;
	FRotator sEshot1;
	FRotator sEshot2;
	FRotator sEshot3;
	FRotator sEshot4;
	FRotator sEshot5;
	FRotator sEshot6;
	FRotator sEshot7;
	FRotator sEshot8;
	// 부수는 벽 관련 
	FVector BulletLoc;
	FRotator BulletRot;
	int		bulletWallid;
	bool	bBulletWall = false;
	// 아이템 삭제 여부
	bool	bDestroyItem = false;
	// 라이트 on/off
	bool	bLightOn = false;
	/////////////////
	FVector FMyLocation;
	FVector FMyDirection;
	FRotator FEffect;
	PlayerType p_type;
	WeaponType w_type;
	///-----------------
	FVector CH1NiaLoc;
	friend ostream& operator<<(ostream& stream, CPlayer& info)
	{
		stream << info.Id << endl;
		stream << info.X << endl;
		stream << info.Y << endl;
		stream << info.Z << endl;
		stream << info.VeloX << endl;
		stream << info.VeloY << endl;
		stream << info.VeloZ << endl;
		stream << info.Yaw << endl;
		stream << info.Pitch << endl;
		stream << info.Roll << endl;
		return stream;
	}

	friend istream& operator>>(istream& stream, CPlayer& info)
	{
		stream >> info.Id;
		stream >> info.X;
		stream >> info.Y;
		stream >> info.Z;
		stream >> info.VeloX;
		stream >> info.VeloY;
		stream >> info.VeloZ;
		stream >> info.Yaw;
		stream >> info.Pitch;
		stream >> info.Roll;
		return stream;
	}
};

class CPlayerInfo
{
public:
	CPlayerInfo() {};
	~CPlayerInfo() {};

	map<int, CPlayer> players;

	friend ostream& operator<<(ostream& stream, CPlayerInfo& info)
	{
		stream << info.players.size() << endl;
		for (auto& kvp : info.players)
		{
			stream << kvp.first << endl;
			stream << kvp.second << endl;
		}

		return stream;
	}

	friend istream& operator>>(istream& stream, CPlayerInfo& info)
	{
		int nPlayers = 0;
		int SessionId = 0;
		CPlayer Player;
		info.players.clear();

		stream >> nPlayers;
		for (int i = 0; i < nPlayers; i++)
		{
			stream >> SessionId;
			stream >> Player;
			info.players[SessionId] = Player;
		}

		return stream;
	}
};

class CItem
{
public:
	CItem() { };
	~CItem() {};

	// 세션 아이디
	int Id = -1;
	//int hp;
	// 위치
	float X = 0;
	float Y = 0;
	float Z = 0;
	// 속도
	friend ostream& operator<<(ostream& stream, CItem& info)
	{
		stream << info.Id << endl;
		stream << info.X << endl;
		stream << info.Y << endl;
		stream << info.Z << endl;
		return stream;
	}

	friend istream& operator>>(istream& stream, CItem& info)
	{
		stream >> info.Id;
		stream >> info.X;
		stream >> info.Y;
		stream >> info.Z;
		return stream;
	}
};

class CItemInfo
{
public:
	CItemInfo() {};
	~CItemInfo() {};

	map<int, CItem> items;

	friend ostream& operator<<(ostream& stream, CItemInfo& info)
	{
		stream << info.items.size() << endl;
		for (auto& kvp : info.items)
		{
			stream << kvp.first << endl;
			stream << kvp.second << endl;
		}

		return stream;
	}

	friend istream& operator>>(istream& stream, CItemInfo& info)
	{
		int nitems = 0;
		int SessionId = 0;
		CItem item;
		info.items.clear();

		stream >> nitems;
		for (int i = 0; i < nitems; i++)
		{
			stream >> SessionId;
			stream >> item;
			info.items[SessionId] = item;
		}

		return stream;
	}
};

struct CharacterInfo {
	FString nickname;
	int clid;
};

class BREAKOUT_API ClientSocket : public FRunnable
{
public:

	ClientSocket();
	virtual ~ClientSocket();
	bool InitSocket();
	bool Connect();
	void CloseSocket();
	bool PacketProcess(char* ptr);
	void Send_Login_Info(char* id, char* pw);
	void Send_Move_Packet(int sessionID, FVector Location, FRotator Rotation, FVector Velocity, float Max_speed, float AO_Yaw, float AO_Pitch);
	void Send_Character_Type(PlayerType type, int id);
	void Send_Weapon_Type(WeaponType type, int id);
	void Send_Ready_Packet(bool ready, int id);
	void Send_Fire_Effect(int attack_id, FVector ImLoc, FRotator ImRot, int wtype);
	void Send_BojoWeapon_packet(int id, FVector ImLoc, FRotator ImRot, int wtype);
	void Send_ShotGun_packet(int attack_id, FVector ServerBeamStart, TArray<FRotator> ServerBeamEnd, int size);
	void Send_Niagara_packet(int clientid, PlayerType type, int num);
	void Send_Niagara_cancel(bool bcancel, int id, int num);
	void Send_Niagara_packetch1(int clinetid, PlayerType type, FVector loc, int num);
	void Send_Start_game_packet(int id);
	void Send_item_info_packet(int objid);
	void Send_End_Game_packet(int id, bool bPlay);
	void Send_Signal_packet(int id, int num);
	void Send_Item_packet(int id, int itemCount);
	void Send_Alive_packet(int id, int deadtype);
	void Send_Destroyed_item_packet(int id, int playerid);
	void Send_Increase_item_count_packet(int id, int itemcount);
	void Send_Decrease_item_count_packet(int id, int itemcount);
	void Send_Reload_packet(int id, bool bReload);
	void Send_item_Anim_packet(int id, int num);
	void Send_Remove_Weapon(int id, bool bWeapon);
	void Send_CH2_SKILL_PACKET(int id, PlayerType type, bool bSkill);
	void Send_My_HP_PACKET(int id, float damaage ,bool bAlive);
	void Send_Dissolve_packet(int id, int dissolve);
	void Send_BojoAnim_packet(int id, int bojo);
	void Send_Mopp_Sync_packet(int itemid, int mopptype, int id);
	void Send_BulletWall_packet(int id, int Wallid, FVector WLoc, FRotator WRot);
	void Send_Light_On_packet(int id, bool bLight);
	void Send_Lobby_Room_pakcet(int id, int RoomNum);
	void Send_Hovered_packet(int id, int RoomNum);
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	char    _id[MAX_NAME_SIZE];
	char    _pw[MAX_NAME_SIZE];
	void RecvPacket();
	void SendPacket(void* packet);
	bool StartListen();
	//void StopListen();

	static ClientSocket* GetSingleton() {
		static ClientSocket ins;
		return &ins;
	}
	void SetGameInstance(UBOGameInstance* inst) { gameinst = inst; }
	void SetPlayerController(ACharacterController* CharacterController);
	HANDLE Iocp;
	Overlap _recv_over;
	SOCKET ServerSocket;
	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;
	int _prev_size = 0;
	int local_id = -1;
	bool login_cond = false;
	bool bAllReady = false;
	bool itemflag = false;
	int MyItemCount;
	std::queue<std::shared_ptr<CItem>> ItemQueue;
	queue<FString>Tempname;
	//--------------
	int tempid;
	int tempid2;
	int Tempcnt;
	int Tempcnt2;
	//-------------
	FString TempPlayerName;
	bool bAcquire = false;
	bool bTravel = false;
	bool bName = false;
	int MoppType = -1;

	//로비룸 이동
	bool binLobby = false;
	// endgame
	bool bEndGame = false;
	bool bitemcount = false;
	inline float get_hp()
	{
		return hp;
	}

	char* IP_addr = nullptr;

private:
	ACharacterController* MyCharacterController;
	CPlayerInfo PlayerInfo;
	UBOGameInstance* gameinst;
	CItemInfo Iteminfo;
	float hp = 100.f;
};

