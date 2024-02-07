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

void lOCPServer::Start()
{

	iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	cout << "서버 시작" << endl;
	if (!CreateWorkerThreads()) return;
	while (true)
	{
		PostAccept();
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
	while (true)
	{
		DWORD bytesTransferred;
		ULONG_PTR completionKey;
		Overlapped* overlapped;

		// IO 완료 패킷을 기다립니다.
		if (!GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &completionKey, (LPOVERLAPPED*)&overlapped, INFINITE))
		{
			// 에러 처리 (추가적인 에러 처리를 추가할 수 있음)
			cout << "GetQueuedCompletionStatus failed with error: " << GetLastError() << endl;
			cout << "??";
			continue;
		}
		// IO 작업 유형을 결정하기 위해 완료 키를 확인
		switch (overlapped->type)
		{
		case IO_RECV:
			// 수신 완료 처리
			HandleReceive(overlapped, bytesTransferred);
			break;
		case IO_SEND:
			// 송신 완료 처리
			HandleSend(overlapped, bytesTransferred);
			break;
		case IO_ACCEPT:
			// Accept 완료 처리
			HandleAccept(overlapped, bytesTransferred);
			break;
			// 필요에 따라 더 많은 경우를 추가 (예: IO_CONNECT)
		default:
			cout << "Unknown IO type" << endl;
			break;
		}
	}
}


void lOCPServer::HandleReceive(Overlapped* overlapped, DWORD bytesTransferred)
{
	// 수신된 데이터 처리, 패킷 처리 등을 수행
	// overlapped->recvBuffer에서 수신된 데이터에 액세스할 수 있습니다.

	// 예시: 수신된 데이터 출력
	cout << "Received data: " << overlapped->recvBuffer << endl;

	// 계속해서 새로운 수신 작업을 등록
	PostRecv(overlapped);
}

void lOCPServer::HandleSend(Overlapped* overlapped, DWORD bytesTransferred)
{
	// 송신 작업 완료 처리, 필요에 따라 추가 로직 구현
	// ...
	cout << "Send data: " << overlapped->recvBuffer << endl;
	PostSend(overlapped);
	// 필요한 경우 리소스 정리, 소켓 닫기 등을 수행
}

void lOCPServer::HandleAccept(Overlapped* overlapped, DWORD bytesTransferred)
{
	// Accept 작업 완료 처리, 필요에 따라 새로운 워커 스레드 생성 또는 수락된 소켓 처리 등을 수행
	// ...
	//cout << "Send data: " << overlapped->recvBuffer << endl;
	// 새로운 Accept 작업을 등록하여 계속해서 들어오는 연결을 수신
	cout << "연결됨" << endl;
	PostAccept();
}


void lOCPServer::PostRecv(Overlapped* overlapped)
{
	// 새로운 수신 작업을 준비하고 등록
	DWORD flags = 0;
	DWORD recvBytes = 0;

	int result = WSARecv(overlapped->socket, &(overlapped->wsabuf), 1, &recvBytes, &flags, &(overlapped->overlapped), NULL);

	if (result == SOCKET_ERROR && (WSAGetLastError() != WSA_IO_PENDING))
	{
		cout << "WSARecv failed with error: " << WSAGetLastError() << endl;
	}
}

void lOCPServer::PostSend(Overlapped* overlapped)
{
	// 새로운 송신 작업을 준비하고 등록
	DWORD sendBytes = 0;

	int result = WSASend(overlapped->socket, &(overlapped->wsabuf), 1, &sendBytes, 0, &(overlapped->overlapped), NULL);

	if (result == SOCKET_ERROR && (WSAGetLastError() != WSA_IO_PENDING))
	{
		cout << "WSASend failed with error: " << WSAGetLastError() << endl;
	}
}


void lOCPServer::PostAccept()
{
	int result;
	// 클라이언트 정보
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	DWORD recvBytes;
	DWORD flags;
	SOCKET clientsocket = WSAAccept(listensocket, (struct sockaddr*)&clientAddr, &addrLen, NULL, NULL);
	if (clientsocket == INVALID_SOCKET) {
		cout << "accept 실패" << endl;
		return;
	}
	Overlapped* overlap = new Overlapped();
	overlap->socket = clientsocket;
	overlap->recvBytes = 0;
	overlap->sendBytes = 0;
	overlap->wsabuf.len = 1024;
	overlap->wsabuf.buf = overlap->recvBuffer;
	flags = 0;
	::CreateIoCompletionPort((HANDLE)clientsocket, iocpHandle, (DWORD)overlap, 0);

	// 클라이언트에게서 첫 번째 패킷을 받기 위해 Recv 호출
	PostRecv(overlap);
}