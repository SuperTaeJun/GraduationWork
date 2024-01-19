#include "pch.h"
#include "lOCPServer.h"

lOCPServer::lOCPServer()
{
	bAccept = true;			// 요청 동작 플래그
	bWorkerThread = true;	// 작업 스레드 동작 플래그

}

lOCPServer::~lOCPServer()
{
	WSACleanup();
	cout << "종료" << endl;
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
	// 클라이언트 정보
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	DWORD recvBytes;
	DWORD flags;
	iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	cout << "서버 시작" << endl;
	if (!CreateWorkerThreads()) return;

	while (bAccept)
	{
		SOCKET clientsocket = WSAAccept(listensocket, (struct sockaddr*)&clientAddr, &addrLen, NULL, NULL);
		if (clientsocket == INVALID_SOCKET) {
			cout << "accept 실패" << endl;
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

	// 최소 1개의 워커 스레드를 생성
	int WorkerCount = numCores;
	// 워커 스레드 생성
	for (int i = 0; i < WorkerCount; ++i) {
		workerthread.emplace_back([this]() {WorkerThread(); });

	}


	std::cout << "Created " << WorkerCount << " worker threads." << std::endl;
	return true;
}

void lOCPServer::WorkerThread()
{
	
	/*cout << "실행 중 " << endl;*/
	// 함수 호출 성공 여부
	BOOL    bResult;
	int     nResult;
	// Overlapped I/O 작업에서 전송된 데이터 크기
	DWORD   recvBytes;
	DWORD   sendBytes;
	// Completion Key를 받을 포인터 변수
	Overlapped* pCompletionKey;
	// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터    
	Overlapped* overlap;
	// 
	DWORD   dwFlags = 0;

	while (bWorkerThread) {
		/*
		* 이 함수로 인해 쓰레드들은 WaitingThread Queue 에 대기상태로 들어가게 됨
		* 완료된 Overlapped I/O 작업이 발생하면 IOCP Queue 에서 완료된 작업을 가져와
		* 뒷처리를 함
		*/
		bResult = GetQueuedCompletionStatus(iocpHandle,
			&recvBytes,                // 실제로 전송된 바이트
			(PULONG_PTR)&pCompletionKey,    // completion key
			(LPOVERLAPPED*)&overlap,            // overlapped I/O 객체
			INFINITE                // 대기할 시간
		);
		if (recvBytes == 0)
			cout << "??" << endl;

		if (recvBytes > 0) {
			cout << "[INFO] 소켓(" << overlap->socket << ")로부터 데이터 수신: " << overlap->wsabuf.buf << endl;

			// todo

			// 추가 데이터를 수신하기 위해 다시 WSARecv 작업을 시작
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
				cout << "[ERROR] WSARecv 실패: " << WSAGetLastError() << endl;
				closesocket(overlap->socket);
				delete overlap;
				continue;
			}
		}
		else {
			// 0 바이트를 받음 - 연결이 닫힐 수 있으므로 이에 대한 처리를 수행
			cout << "[INFO] 소켓(" << overlap->socket << ")로부터 0 바이트 수신" << endl;
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
//			cout << "받은거 없음" << endl;
//		if (recvBytes > 0)
//		{
//			cout << "[INFO] 소켓(" << overlapped->socket << ")로부터 데이터 수신: " << overlapped->wsabuf.buf << endl;
//
//			// todo
//
//			// 추가 데이터를 수신하기 위해 다시 WSARecv 작업을 시작
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
//			// TODO : 연결 끊김
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