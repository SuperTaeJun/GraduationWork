#pragma once
#include "Overlapped.h"
class lOCPServer
{
public:
	lOCPServer();
	virtual ~lOCPServer();
	bool Init();
	bool Start();
	void CreateWorkerThreads();
	void WorkerThread();
protected:
	SOCKET listensocket;
	HANDLE iocpHandle;
	Overlapped* overlap;
public:
	vector <thread> workerthread;
};

