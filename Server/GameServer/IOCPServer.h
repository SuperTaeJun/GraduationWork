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

	// ���� ��� �� ���� ���� ����
	bool Initialize();
	// ���� ����
	void StartServer();
	// �۾� ������ ����
	bool CreateWorkerThread();
	// �۾� ������
	void WorkerThread();

	

private:
	stSOCKETINFO* pSocketInfo;		// ���� ����
	SOCKET			listenSocket;		// ���� ���� ����
	HANDLE			hIOCP;			// IOCP ��ü �ڵ�
	bool			bAccept;			// ��û ���� �÷���
	bool			bWorkerThread;	// �۾� ������ ���� �÷���
	HANDLE* hWorkerHandle;	// �۾� ������ �ڵ�
	map<int, SOCKET> SessionSocket;
};
