#include "pch.h"
#include "lOCPServer.h"

lOCPServer::lOCPServer()
{
	bAccept = true;			// ��û ���� �÷���
	bWorkerThread = true;	// �۾� ������ ���� �÷���

}

lOCPServer::~lOCPServer()
{
	WSACleanup();
	cout << "����" << endl;
}

bool lOCPServer::Init()
{
	WSADATA wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;
	listensocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (listensocket == INVALID_SOCKET)
		return 0;
	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(9000);
	if (::bind(listensocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;
	if (::listen(listensocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;
	return true;
}

void lOCPServer::Start()
{
	int result;
	// Ŭ���̾�Ʈ ����
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	DWORD recvBytes;
	DWORD flags;
	iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	cout << "���� ����" << endl;
	if (!CreateWorkerThreads()) return;

	while (bAccept)
	{
		SOCKET clientsocket = WSAAccept(listensocket, (struct sockaddr*)&clientAddr, &addrLen, NULL, NULL);
		if (clientsocket == INVALID_SOCKET) {
			cout << "accept ����" << endl;
			return;
		}
		overlap = new Overlapped();
		overlap->socket = clientsocket;
		overlap->recvBytes = 0;
		overlap->sendBytes = 0;
		overlap->wsabuf.len = 1024;
		overlap->wsabuf.buf = overlap->recvBuffer;
		flags = 0;
		::CreateIoCompletionPort((HANDLE)clientsocket, iocpHandle, (DWORD)overlap, 0);

		result = WSARecv(
			overlap->socket,
			&overlap->wsabuf,
			1,
			&recvBytes,
			&flags,
			&(overlap->overlapped),
			NULL
		);
	}

}

bool lOCPServer::CreateWorkerThreads()
{
	unsigned int numCores = thread::hardware_concurrency();

	// �ּ� 1���� ��Ŀ �����带 ����
	int WorkerCount = numCores;
	// ��Ŀ ������ ����
	for (int i = 0; i < WorkerCount; ++i) {
		workerthread.emplace_back([this]() {WorkerThread(); });

	}


	std::cout << "Created " << WorkerCount << " worker threads." << std::endl;
	return true;
}

void lOCPServer::WorkerThread()
{
	
	/*cout << "���� �� " << endl;*/
	// �Լ� ȣ�� ���� ����
	BOOL    bResult;
	int     nResult;
	// Overlapped I/O �۾����� ���۵� ������ ũ��
	DWORD   recvBytes;
	DWORD   sendBytes;
	// Completion Key�� ���� ������ ����
	Overlapped* pCompletionKey;
	// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������    
	Overlapped* overlap;
	// 
	DWORD   dwFlags = 0;

	while (bWorkerThread) {
		/*
		* �� �Լ��� ���� ��������� WaitingThread Queue �� �����·� ���� ��
		* �Ϸ�� Overlapped I/O �۾��� �߻��ϸ� IOCP Queue ���� �Ϸ�� �۾��� ������
		* ��ó���� ��
		*/
		bResult = GetQueuedCompletionStatus(iocpHandle,
			&recvBytes,                // ������ ���۵� ����Ʈ
			(PULONG_PTR)&pCompletionKey,    // completion key
			(LPOVERLAPPED*)&overlap,            // overlapped I/O ��ü
			INFINITE                // ����� �ð�
		);
		if (recvBytes == 0)
			cout << "??" << endl;

		if (recvBytes > 0) {
			cout << "[INFO] ����(" << overlap->socket << ")�κ��� ������ ����: " << overlap->wsabuf.buf << endl;

			// todo

			// �߰� �����͸� �����ϱ� ���� �ٽ� WSARecv �۾��� ����
			overlap->recvBytes = 0;
			DWORD flags = 0;
			nResult = WSARecv(
				overlap->socket,
				&overlap->wsabuf,
				1,
				&recvBytes,
				&flags,
				&(overlap->overlapped),
				NULL
			);

			if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
				cout << "[ERROR] WSARecv ����: " << WSAGetLastError() << endl;
				closesocket(overlap->socket);
				delete overlap;
				continue;
			}
		}
		else {
			// 0 ����Ʈ�� ���� - ������ ���� �� �����Ƿ� �̿� ���� ó���� ����
			cout << "[INFO] ����(" << overlap->socket << ")�κ��� 0 ����Ʈ ����" << endl;
			closesocket(overlap->socket);
			delete overlap;
		}
		// todo
	}

}

//void lOCPServer::WorkerThread()
//{
//	// ...
//	DWORD   recvBytes;
//	DWORD   sendBytes;
//	Overlapped* completionKey;
//	Overlapped* overlapped;
//	while (true)
//	{
//		BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &recvBytes,
//			(ULONG_PTR*)&completionKey, (LPOVERLAPPED*)&overlapped, INFINITE);
//		if (ret == FALSE)
//			continue;
//
//		if (recvBytes == 0)
//			cout << "������ ����" << endl;
//		if (recvBytes > 0)
//		{
//			cout << "[INFO] ����(" << overlapped->socket << ")�κ��� ������ ����: " << overlapped->wsabuf.buf << endl;
//
//			// todo
//
//			// �߰� �����͸� �����ϱ� ���� �ٽ� WSARecv �۾��� ����
//			overlapped->recvBytes = 0;
//			DWORD flags = 0;
//			int result = WSARecv(
//				overlapped->socket,
//				&overlapped->wsabuf,
//				1,
//				&recvBytes,
//				&flags,
//				&(overlapped->overlapped),
//				NULL
//			);
//		}
//	}
//}
//void lOCPServer::WorkerThread()
//{
//	while (true)
//	{
//		DWORD bytesTransferred = 0;
//		//Session* session = nullptr;
//		Overlapped* overlappedEx = nullptr;
//
//		BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred,
//			(ULONG_PTR*)&overlappedEx, (LPOVERLAPPED*)&overlappedEx, INFINITE);
//
//		if (ret == FALSE || bytesTransferred == 0)
//		{
//			// TODO : ���� ����
//			continue;
//		}
//		overlappedEx->type == IO_type::IO_RECV;
//		cout << "Recv Data IOCP = " << bytesTransferred << endl;
//		WSABUF wsaBuf;
//		wsaBuf.buf = overlappedEx->recvBuffer;
//		wsaBuf.len = 1000;
//
//		DWORD recvLen = 0;
//		DWORD flags = 0;
//		::WSARecv(overlappedEx->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
//	}
//}