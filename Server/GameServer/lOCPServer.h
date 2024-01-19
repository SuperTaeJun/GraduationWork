#pragma once
#include "Overlapped.h"
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
	bool	bAccept;			// ��û ���� �÷���
	bool	bWorkerThread;	// �۾� ������ ���� �÷���
public:
	vector <thread> workerthread;
};

