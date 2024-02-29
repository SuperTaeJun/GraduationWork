// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <fstream>
#include <map>
#include <queue>
#include <iostream>
#include "Player/CharacterController.h"
#include "Network/PacketData.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "CoreMinimal.h"




class ClientSocket;
class ABOGameMode;
class ACharacterController;
using namespace std;
enum OPTYPE {
	OP_SEND,
	OP_RECV,
};


const int buffsize = 1000;

enum IO_type
{
	IO_RECV,
	IO_SEND,
	IO_ACCEPT,
	//IO_CONNECT,
};

class Overlapped {
public:
	WSAOVERLAPPED	overlapped;
	WSABUF			wsabuf;
	SOCKET			socket;
	unsigned char			recvBuffer[buffsize + 1];
	int				recvBytes;
	int				sendBytes;
	IO_type			type; // read, write, accept, connect ...
public:
	Overlapped(IO_type type, char bytes, void* mess) : type(type)
	{
		ZeroMemory(&overlapped, sizeof(overlapped));
		wsabuf.buf = reinterpret_cast<char*>(recvBuffer);
		wsabuf.len = bytes;
		memcpy(recvBuffer, mess, bytes);
	}
	Overlapped(IO_type type) : type(type)
	{
		ZeroMemory(&overlapped, sizeof(overlapped));
		wsabuf.buf = {};
		wsabuf.len = {};
	}
	Overlapped()
	{
		type = IO_RECV;
		ZeroMemory(&overlapped, sizeof(overlapped));
		wsabuf.buf = {};
		wsabuf.len = {};
	}
	~Overlapped()
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
	// 아이디 비번
	char	userId[20] = {};
	char	userPw[20] = {};
	// 위치
	float X = 0;
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
	FVector FMyLocation;
	FVector FMyDirection;
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

/**
 *
 */
class BREAKOUT_API ClientSocket : public FRunnable
{
public:
	ClientSocket();
	virtual ~ClientSocket();
	bool InitSocket();
	bool Connect(const char* s_IP, int port);
	void CloseSocket();
	
	void PacketProcess(unsigned char* ptr);
	void Send_Login_Info(char* id, char* pw);
	void Send_Move_Packet(int sessionID, float x, float y, float z);
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();

	// 스레드 시작 및 종료
	bool StartListen();
	void StopListen();
	void RecvPacket();
	void SendPacket(void* packet);
	// 싱글턴 객체 가져오기
	static ClientSocket* GetSingleton() {
		static ClientSocket ins;
		return &ins;
	}
	void SetPlayerController(ACharacterController* CharacterController);
	HANDLE Iocp;
	Overlapped _recv_over;
	int      _prev_size = 0;
	SOCKET ServerSocket;
	char recvBuffer[MAX_BUFFER];
	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;

	bool login_cond = false;
private:
	ACharacterController* MyCharacterController;
	CPlayerInfo PlayerInfo;
};