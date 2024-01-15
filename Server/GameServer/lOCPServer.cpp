#include "pch.h"
#include "lOCPServer.h"

lOCPServer::lOCPServer()
{
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
	serverAddr.sin_port = ::htons(12345);
	if (::bind(listensocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		return 0;
	if (::listen(listensocket, SOMAXCONN) == SOCKET_ERROR)
		return 0;
	return true;
}

bool lOCPServer::Start()
{
	iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	SOCKET acceptSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (acceptSocket == INVALID_SOCKET)
		return false;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listensocket), iocpHandle, 0, 0);
	Overlapped* overlappedData = new Overlapped();
	memset(overlappedData, 0, sizeof(Overlapped));
	overlappedData->type = IO_ACCEPT;
	DWORD bytesReceived;
	cout << "���� ����" << endl;
	if (!AcceptEx(listensocket, acceptSocket, overlappedData->messageBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesReceived, &(overlappedData->overlapped)))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			// ���� ó��
			cout << "??";
			delete overlappedData;
			closesocket(acceptSocket);
			return false;
		}
		
	}

	return true;
}

void lOCPServer::CreateWorkerThreads()
{
	unsigned int numCores = thread::hardware_concurrency();

	// �ּ� 1���� ��Ŀ �����带 ����
	int WorkerCount = numCores;
	// ��Ŀ ������ ����
	for (int i = 0; i < WorkerCount; ++i)
		workerthread.emplace_back([this]() {WorkerThread(); });

	std::cout << "Created " << WorkerCount << " worker threads." << std::endl;
}

//void lOCPServer::WorkerThread()
//{
//	//// �Լ� ȣ�� ���� ����
//	//BOOL    bResult;
//	//int     nResult;
//	//// Overlapped I/O �۾����� ���۵� ������ ũ��
//	//DWORD   recvBytes;
//	//DWORD   sendBytes;
//	//// Completion Key�� ���� ������ ����
//	//Overlapped* pCompletionKey;
//	//// I/O �۾��� ���� ��û�� Overlapped ����ü�� ���� ������    
//	//Overlapped* pSocketInfo;
//	//// 
//	//DWORD   dwFlags = 0;
//
//	//while (true) {
//	//	/*
//	//	* �� �Լ��� ���� ��������� WaitingThread Queue �� �����·� ���� ��
//	//	* �Ϸ�� Overlapped I/O �۾��� �߻��ϸ� IOCP Queue ���� �Ϸ�� �۾��� ������
//	//	* ��ó���� ��
//	//	*/
//	//	bResult = GetQueuedCompletionStatus(iocpHandle,
//	//		&recvBytes,                // ������ ���۵� ����Ʈ
//	//		(PULONG_PTR)&pCompletionKey,    // completion key
//	//		(LPOVERLAPPED*)&pSocketInfo,            // overlapped I/O ��ü
//	//		INFINITE                // ����� �ð�
//	//	);
//
//	//	if (recvBytes > 0) {
//	//		cout << "[INFO] ����(" << pSocketInfo->socket << ")�κ��� ������ ����: " << pSocketInfo->dataBuf.buf << endl;
//
//	//		// todo
//
//	//		// �߰� �����͸� �����ϱ� ���� �ٽ� WSARecv �۾��� ����
//	//		pSocketInfo->recvBytes = 0;
//	//		DWORD flags = 0;
//	//		nResult = WSARecv(
//	//			pSocketInfo->socket,
//	//			&pSocketInfo->dataBuf,
//	//			1,
//	//			&recvBytes,
//	//			&flags,
//	//			&(pSocketInfo->overlapped),
//	//			NULL
//	//		);
//
//	//		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
//	//			cout << "[ERROR] WSARecv ����: " << WSAGetLastError() << endl;
//	//			closesocket(pSocketInfo->socket);
//	//			free(pSocketInfo);
//	//			continue;
//	//		}
//	//	}
//	//	else {
//	//		// 0 ����Ʈ�� ���� - ������ ���� �� �����Ƿ� �̿� ���� ó���� ����
//	//		cout << "[INFO] ����(" << pSocketInfo->socket << ")�κ��� 0 ����Ʈ ����" << endl;
//	//		closesocket(pSocketInfo->socket);
//	//		free(pSocketInfo);
//	//	}
//	//	// todo
//	//}
//    
//}

void lOCPServer::WorkerThread()
{
	// �Լ� ȣ�� ���� ����
	BOOL    bResult;
	int     nResult;
	// Overlapped I/O �۾����� ���۵� ������ ũ��
	DWORD   recvBytes;
	DWORD   sendBytes;
	// Completion Key�� ���� ������ ����
	Overlapped* completionKey;
	Overlapped* overlapped;
	while (true)
	{
		//cout << "���� ����?" << endl;
		BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &recvBytes,
			(ULONG_PTR*)&completionKey, (LPOVERLAPPED*)&overlapped, INFINITE);
		if (ret == FALSE)
			continue;
		if (recvBytes > 0) {
			cout << "[INFO] ����(" << overlapped->socket << ")�κ��� ������ ����: " << overlapped->dataBuf.buf << endl;

			// todo

			// �߰� �����͸� �����ϱ� ���� �ٽ� WSARecv �۾��� ����
			overlapped->recvBytes = 0;
			DWORD flags = 0;
			nResult = WSARecv(
				overlapped->socket,
				&overlapped->dataBuf,
				1,
				&recvBytes,
				&flags,
				&(overlapped->overlapped),
				NULL
			);
		}
	}
}