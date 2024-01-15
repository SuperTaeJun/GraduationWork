#include "pch.h"
#include "lOCPServer.h"

lOCPServer::lOCPServer()
{
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
	cout << "서버 시작" << endl;
	if (!AcceptEx(listensocket, acceptSocket, overlappedData->messageBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesReceived, &(overlappedData->overlapped)))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			// 에러 처리
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

	// 최소 1개의 워커 스레드를 생성
	int WorkerCount = numCores;
	// 워커 스레드 생성
	for (int i = 0; i < WorkerCount; ++i)
		workerthread.emplace_back([this]() {WorkerThread(); });

	std::cout << "Created " << WorkerCount << " worker threads." << std::endl;
}

//void lOCPServer::WorkerThread()
//{
//	//// 함수 호출 성공 여부
//	//BOOL    bResult;
//	//int     nResult;
//	//// Overlapped I/O 작업에서 전송된 데이터 크기
//	//DWORD   recvBytes;
//	//DWORD   sendBytes;
//	//// Completion Key를 받을 포인터 변수
//	//Overlapped* pCompletionKey;
//	//// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터    
//	//Overlapped* pSocketInfo;
//	//// 
//	//DWORD   dwFlags = 0;
//
//	//while (true) {
//	//	/*
//	//	* 이 함수로 인해 쓰레드들은 WaitingThread Queue 에 대기상태로 들어가게 됨
//	//	* 완료된 Overlapped I/O 작업이 발생하면 IOCP Queue 에서 완료된 작업을 가져와
//	//	* 뒷처리를 함
//	//	*/
//	//	bResult = GetQueuedCompletionStatus(iocpHandle,
//	//		&recvBytes,                // 실제로 전송된 바이트
//	//		(PULONG_PTR)&pCompletionKey,    // completion key
//	//		(LPOVERLAPPED*)&pSocketInfo,            // overlapped I/O 객체
//	//		INFINITE                // 대기할 시간
//	//	);
//
//	//	if (recvBytes > 0) {
//	//		cout << "[INFO] 소켓(" << pSocketInfo->socket << ")로부터 데이터 수신: " << pSocketInfo->dataBuf.buf << endl;
//
//	//		// todo
//
//	//		// 추가 데이터를 수신하기 위해 다시 WSARecv 작업을 시작
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
//	//			cout << "[ERROR] WSARecv 실패: " << WSAGetLastError() << endl;
//	//			closesocket(pSocketInfo->socket);
//	//			free(pSocketInfo);
//	//			continue;
//	//		}
//	//	}
//	//	else {
//	//		// 0 바이트를 받음 - 연결이 닫힐 수 있으므로 이에 대한 처리를 수행
//	//		cout << "[INFO] 소켓(" << pSocketInfo->socket << ")로부터 0 바이트 수신" << endl;
//	//		closesocket(pSocketInfo->socket);
//	//		free(pSocketInfo);
//	//	}
//	//	// todo
//	//}
//    
//}

void lOCPServer::WorkerThread()
{
	// 함수 호출 성공 여부
	BOOL    bResult;
	int     nResult;
	// Overlapped I/O 작업에서 전송된 데이터 크기
	DWORD   recvBytes;
	DWORD   sendBytes;
	// Completion Key를 받을 포인터 변수
	Overlapped* completionKey;
	Overlapped* overlapped;
	while (true)
	{
		//cout << "여기 들어옴?" << endl;
		BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &recvBytes,
			(ULONG_PTR*)&completionKey, (LPOVERLAPPED*)&overlapped, INFINITE);
		if (ret == FALSE)
			continue;
		if (recvBytes > 0) {
			cout << "[INFO] 소켓(" << overlapped->socket << ")로부터 데이터 수신: " << overlapped->dataBuf.buf << endl;

			// todo

			// 추가 데이터를 수신하기 위해 다시 WSARecv 작업을 시작
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