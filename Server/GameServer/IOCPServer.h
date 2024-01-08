#pragma once
#include <iostream>
using namespace std;
struct stSOCKETINFO {
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuf;
	SOCKET			socket;
	char			messageBuffer[1024];
	int				recvBytes;
	int				sendBytes;
};
class IOCompletionPort {
public:
	IOCompletionPort();
	~IOCompletionPort();

	// 소켓 등록 및 서버 정보 설정
	bool Initialize();
	// 서버 시작
	void StartServer();
	// 작업 스레드 생성
	bool CreateWorkerThread();
	// 작업 스레드
	void WorkerThread();

	

private:
	stSOCKETINFO* pSocketInfo;		// 소켓 정보
	SOCKET			listenSocket;		// 서버 리슨 소켓
	HANDLE			hIOCP;			// IOCP 객체 핸들
	bool			bAccept;			// 요청 동작 플래그
	bool			bWorkerThread;	// 작업 스레드 동작 플래그
	HANDLE* hWorkerHandle;	// 작업 스레드 핸들
	map<int, SOCKET> SessionSocket;
};
