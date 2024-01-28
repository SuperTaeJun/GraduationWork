// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <fstream>
#include <iostream>
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "CoreMinimal.h"


#define	MAX_BUFFER		4096
#define SERVER_PORT		12345
#define SERVER_IP		"127.0.0.1"
#define MAX_CLIENTS		100

class ABOGameMode;
class ACharacterController;
using namespace std;

enum OPTYPE {
	OP_SEND, 
	OP_RECV, 
};

class Overlap {
public:
	WSAOVERLAPPED   _wsa_over;
	OPTYPE         _op;
	WSABUF         _wsa_buf;
	unsigned char   _net_buf[1024];
	int32            _target;
public:
	Overlap(OPTYPE _op, char num_bytes, void* mess) : _op(_op)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
		_wsa_buf.len = num_bytes;
		memcpy(_net_buf, mess, num_bytes);
	}

	Overlap(OPTYPE _op) : _op(_op) {}

	Overlap()
	{
		_op = OP_RECV;
	}

	~Overlap()
	{
	}
};



// 플레이어 클래스 
class Player
{
public:
	Player() { };
	~Player() {};

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

	friend ostream& operator<<(ostream& stream, Player& info)
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

	friend istream& operator>>(istream& stream, Player& info)
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
	
	void PacketProcess(const char* ptr);
	void Send_Login_Info(char* id, char* pw);
	
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
	HANDLE Iocp;
	Overlap _recv_over;
	int      _prev_size = 0;
	SOCKET ServerSocket;
	char recvBuffer[MAX_BUFFER];
	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;
private:

};