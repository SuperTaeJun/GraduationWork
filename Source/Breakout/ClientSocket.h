// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "CoreMinimal.h"


#define	MAX_BUFFER		4096
#define SERVER_PORT		7777
#define SERVER_IP		"127.0.0.1"
#define MAX_CLIENTS		100

class ABOGameMode;

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
