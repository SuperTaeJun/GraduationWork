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
	int result;
	// 클라이언트 정보
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	DWORD recvBytes;
	DWORD flags;
	iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	cout << "서버 시작" << endl;
	if (!CreateWorkerThreads()) return;

	while (true)
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
	// Overlapped I/O 작업에서 전송된 데이터 크기
	DWORD   recvBytes;
	DWORD   sendBytes;
	// Completion Key를 받을 포인터 변수
	Overlapped* pCompletionKey;
	// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터    
	Overlapped* overlap;
	DWORD   dwFlags = 0;

	while (true) {
		bool bResult = GetQueuedCompletionStatus(iocpHandle, &recvBytes, (PULONG_PTR)&pCompletionKey, (LPOVERLAPPED*)&overlap, INFINITE);

		if (recvBytes == 0)
			cout << "[INFO] 소켓(" << overlap->socket << ")로부터 0 바이트 수신" << endl;

		if (recvBytes > 0) {
			//cout << "[INFO] 소켓(" << overlap->socket << ")로부터 데이터 수신: " << overlap->wsabuf.buf << endl;

			//if (recvBytes >= sizeof(CS_LOGIN_PACKET)) {
			CS_LOGIN_PACKET* loginPacket = reinterpret_cast<CS_LOGIN_PACKET*>(overlap->wsabuf.buf);

			// loginPacket 처리 (아이디, 비밀번호 확인 등)

			SC_LOGIN_BACK loginOkPacket;
			loginOkPacket.size = sizeof(SC_LOGIN_BACK);
			loginOkPacket.type = 1;  // SC_LOGIN_OK 타입
			loginOkPacket.cl_id = 1; // 세션 아이디 (임의의 값)
			loginOkPacket.x = 0.0f;   // 좌표 (임의의 값)
			loginOkPacket.y = 0.0f;   // 좌표 (임의의 값)
			loginOkPacket.z = 0.0f;   // 좌표 (임의의 값)
			strncpy_s(loginOkPacket.id, MAX_INFO_SIZE, loginPacket->id, MAX_INFO_SIZE);
			strncpy_s(loginOkPacket.pw, MAX_INFO_SIZE, loginPacket->pw, MAX_INFO_SIZE);
			cout << "login info : " << loginOkPacket.id << ", pw:" << loginOkPacket.pw << endl;

			overlap->recvBytes = 0;
			DWORD flags = 0;
			int nResult = WSARecv(
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
	}
}


