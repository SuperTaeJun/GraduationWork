// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <fstream>
#include <map>
#include <mutex>
#include <queue>
#include <iostream>
//#include "Player/CharacterController.h"
#include "Network/PacketData.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "CoreMinimal.h"




class ClientSocket;
class ABOGameMode;
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
	bool  IsAlive = true;
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

const int buffsize = 1000;
const int  MAX_NAME_SIZE = 20;
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
	void Send_Move_Packet(int sessionID, FVector Location, FRotator Rotation, FVector Velocity);
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();
	char	_id[MAX_NAME_SIZE];
	char	_pw[MAX_NAME_SIZE];
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

	SOCKET ServerSocket;
	char recvBuffer[MAX_BUFFER];
	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;

	int _prev_size = 0;
	int local_id = -1;
	bool login_cond = false;
private:
	ACharacterController* MyCharacterController;
	CPlayerInfo PlayerInfo;
	//CPlayer NewPlayer;
};