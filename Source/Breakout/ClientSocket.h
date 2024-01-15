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
	// 소켓이 속한 게임모드를 세팅해주는 함수
	void SetGameMode(ABOGameMode* pGameMode);

	void CloseSocket();

	FRunnableThread* Thread;
	FThreadSafeCounter StopTaskCounter;
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();

	// 스레드 시작 및 종료
	bool StartListen();
	void StopListen();

	// 싱글턴 객체 가져오기
	static ClientSocket* GetSingleton() {
		static ClientSocket ins;
		return &ins;
	}
private:
	SOCKET ServerSocket;
	char recvBuffer[MAX_BUFFER];
	ABOGameMode* GameMode;	// 게임모드 정보
};
