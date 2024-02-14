#pragma once
#include "pch.h"
//#include "PacketHandler.h"
class PacketHandler;
std::unique_ptr<PacketHandler> p_handler;
class lOCPServer
{
public:
	lOCPServer();
	virtual ~lOCPServer();
	bool Init();
	void Start();
	bool CreateWorkerThreads();
	void WorkerThread();
	void HandleReceive(Overlapped* overlapped, DWORD bytesTransferred);
	void HandleSend(Overlapped* overlapped, DWORD bytesTransferred);
	void HandleAccept(Overlapped* overlapped, DWORD bytesTransferred);
	void PostRecv(Overlapped* overlapped);
	void PostSend(Overlapped* overlapped);
	void PostAccept();
	//virtual bool OnRecv(int s_id, Overlapped* overlap, DWORD num_bytes) { return true; };
protected:
	SOCKET listensocket;
	HANDLE iocpHandle;


	bool	bAccept;			// 요청 동작 플래그
	bool	bWorkerThread;	// 작업 스레드 동작 플래그
public:
	vector <thread> workerthread;
};


