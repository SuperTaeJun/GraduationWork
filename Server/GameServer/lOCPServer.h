#pragma once
#include "Overlapped.h"
#include "protocol.h"
class lOCPServer
{
public:
	lOCPServer();
	virtual ~lOCPServer();
	bool Init();
	void Start();
	bool CreateWorkerThreads();
	void WorkerThread();
	virtual bool OnRecv(int s_id, Overlapped* overlap, DWORD num_bytes) { return true; };
protected:
	SOCKET listensocket;
	HANDLE iocpHandle;
	Overlapped* overlap;
	bool	bAccept;			// 요청 동작 플래그
	bool	bWorkerThread;	// 작업 스레드 동작 플래그
public:
	vector <thread> workerthread;
};

