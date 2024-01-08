#include "pch.h"
#include "IOCPServer.h"


unsigned int WINAPI CallWorkerThread(LPVOID p) {
	IOCompletionPort* pOverlappedEvent = (IOCompletionPort*)p;
	pOverlappedEvent->WorkerThread();
	return 0;
}

IOCompletionPort::IOCompletionPort() {
	bWorkerThread = true;
	bAccept = true;

}

IOCompletionPort::~IOCompletionPort() {
	// winsock �� ����� ������
	WSACleanup();
	// �� ����� ��ü�� ����
	if (pSocketInfo) {
		delete[] pSocketInfo;
		pSocketInfo = NULL;
	}
	if (hWorkerHandle) {
		delete[] hWorkerHandle;
		hWorkerHandle = NULL;
	}
}

bool IOCompletionPort::Initialize() {
	WSADATA wsaData;
	int nResult;
	// winsock 2.2 �������� �ʱ�ȭ
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0) {
		cout << "winsock �ʱ�ȭ ����" << endl;
		return false;
	}

	// ���� ����
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET) {
		cout << "���� ���� ����" << endl;
		return false;
	}

	// ���� ���� ����
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(7777);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// ���� ����
	nResult = bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	if (nResult == SOCKET_ERROR) {
		cout <<"bind ����" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// ���� ��⿭ ����
	nResult = listen(listenSocket, 5);
	if (nResult == SOCKET_ERROR) {
		cout << "listen ����" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}
	return true;
}

void IOCompletionPort::StartServer() {
	int nResult;
	// Ŭ���̾�Ʈ ����
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket;
	DWORD recvBytes;
	DWORD flags;

	// Completion Port ��ü ����
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// Worker Thread ����
	if (!CreateWorkerThread()) return;

	cout << "���� ����" << endl;

	// Ŭ���̾�Ʈ ������ ����
	while (bAccept) {
		clientSocket = WSAAccept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen, NULL, NULL);

		if (clientSocket == INVALID_SOCKET) {
			cout << "accept ����" << endl;
			return;
		}

		pSocketInfo = new stSOCKETINFO();
		pSocketInfo->socket = clientSocket;
		pSocketInfo->recvBytes = 0;
		pSocketInfo->sendBytes = 0;
		pSocketInfo->dataBuf.len = 1024;
		pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
		flags = 0;

		hIOCP = CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (DWORD)pSocketInfo, 0);

		// ��ø ������ �����ϰ� �Ϸ�� ����� �Լ��� �Ѱ���
		nResult = WSARecv(
			pSocketInfo->socket,
			&pSocketInfo->dataBuf,
			1,
			&recvBytes,
			&flags,
			&(pSocketInfo->overlapped),
			NULL
		);

		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			cout << "[ERROR] IO Pending ���� : " << WSAGetLastError() << endl;
			return;
		}
	}
}

bool IOCompletionPort::CreateWorkerThread() {
	unsigned int threadId;
	// �ý��� ���� ������
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	cout <<"CPU ���� : " << sysInfo.dwNumberOfProcessors << endl;
	// ������ �۾� �������� ������ (CPU * 2) + 1
	int nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	// thread handler ����
	hWorkerHandle = new HANDLE[nThreadCnt];
	// thread ����
	for (int i = 0; i < nThreadCnt; i++) {
		hWorkerHandle[i] = (HANDLE*)_beginthreadex(NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId);
		if (hWorkerHandle[i] == NULL) {
			cout << "���� ����" << endl;
			return false;
		}
		ResumeThread(hWorkerHandle[i]);
	}
	cout << "����" << endl;
	cout << endl;
	return true;
}

void IOCompletionPort::WorkerThread() {
	// �Լ� ȣ�� ���� ����
	BOOL    bResult;
	int     nResult;
	// Overlapped I/O �۾����� ���۵� ������ ũ��
	DWORD   recvBytes;
	DWORD   sendBytes;
	// Completion Key�� ���� ������ ����
	stSOCKETINFO* pCompletionKey;
	// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������    
	stSOCKETINFO* pSocketInfo;
	// 
	DWORD   dwFlags = 0;

	while (bWorkerThread) {
		/*
		* �� �Լ��� ���� ��������� WaitingThread Queue �� �����·� ���� ��
		* �Ϸ�� Overlapped I/O �۾��� �߻��ϸ� IOCP Queue ���� �Ϸ�� �۾��� ������
		* ��ó���� ��
		*/
		bResult = GetQueuedCompletionStatus(hIOCP,
			&recvBytes,                // ������ ���۵� ����Ʈ
			(PULONG_PTR)&pCompletionKey,    // completion key
			(LPOVERLAPPED*)&pSocketInfo,            // overlapped I/O ��ü
			INFINITE                // ����� �ð�
		);

		if (recvBytes > 0) {
			cout << "[INFO] ����(" << pSocketInfo->socket << ")�κ��� ������ ����: " << pSocketInfo->dataBuf.buf << endl;

			// todo

			// �߰� �����͸� �����ϱ� ���� �ٽ� WSARecv �۾��� ����
			pSocketInfo->recvBytes = 0;
			DWORD flags = 0;
			nResult = WSARecv(
				pSocketInfo->socket,
				&pSocketInfo->dataBuf,
				1,
				&recvBytes,
				&flags,
				&(pSocketInfo->overlapped),
				NULL
			);

			if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
				cout << "[ERROR] WSARecv ����: " << WSAGetLastError() << endl;
				closesocket(pSocketInfo->socket);
				free(pSocketInfo);
				continue;
			}
		}
		else {
			// 0 ����Ʈ�� ���� - ������ ���� �� �����Ƿ� �̿� ���� ó���� ����
			cout << "[INFO] ����(" << pSocketInfo->socket << ")�κ��� 0 ����Ʈ ����" << endl;
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
		}
		// todo
	}
}

