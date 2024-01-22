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

// �÷��̾� Ŭ���� 
class Player
{
public:
	Player() { };
	~Player() {};

	// ���� ���̵�
	int Id = -1;
	// ���̵� ���
	char	userId[20] = {};
	char	userPw[20] = {};
	// ��ġ
	float X = 0;
	float Y = 0;
	float Z = 0;
	// ȸ����
	float Yaw = 0;
	float Pitch = 0;
	float Roll = 0;
	// �ӵ�
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
	// ������ ���� ���Ӹ�带 �������ִ� �Լ�
	void SetGameMode(ABOGameMode* pGameMode);

	void CloseSocket();

	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();

	// ������ ���� �� ����
	bool StartListen();
	void StopListen();

	// �̱��� ��ü ��������
	static ClientSocket* GetSingleton() {
		static ClientSocket ins;
		return &ins;
	}
private:
	SOCKET ServerSocket;
	char recvBuffer[MAX_BUFFER];
	ABOGameMode* GameMode;	// ���Ӹ�� ����
};
