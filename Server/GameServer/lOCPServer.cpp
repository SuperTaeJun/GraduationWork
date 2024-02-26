#include "pch.h"
#include "lOCPServer.h"

lOCPServer::lOCPServer()
{
	//cl_id = 0;
	for (int i = 0; i < 100; ++i) clients[i].cl_id = i;
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
	DWORD dwBytes;
	iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	cout << "서버 시작" << endl;
	if (!CreateWorkerThreads()) return;
	//while (true)
	//{
	//	//PostAccept();
	//}
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listensocket), iocpHandle, 0, 0);
	
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	char   accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
	Overlapped  accept_ex;
	*(reinterpret_cast<SOCKET*>(&accept_ex.recvBuffer)) = c_socket;
	ZeroMemory(&accept_ex.overlapped, sizeof(accept_ex.overlapped));
	accept_ex.type = IO_ACCEPT;
	AcceptEx(listensocket, c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, &accept_ex.overlapped);
	/*for (auto& cl : clients) {
		if (ST_INGAME == cl.cl_state)
			Disconnect(cl.cl_id);
	}
	closesocket(listensocket);*/
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

	//std::cout << "Created " << WorkerCount << " worker threads." << std::endl;
	return true;
}

void lOCPServer::WorkerThread()
{
	while (true)
	{
		//cout << "들어옴";
		DWORD bytesTransferred;
		LONG64 completionKey;
		WSAOVERLAPPED* p_over;

		// IO 완료 패킷을 기다립니다.
		bool ret = GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, 
			(PULONG_PTR)&completionKey, (LPOVERLAPPED*)&p_over, INFINITE);

		int cl_id = static_cast<int>(completionKey);
		Overlapped* overlap = reinterpret_cast<Overlapped*>(p_over);
		if (FALSE == ret) {
			int err_no = WSAGetLastError();
			cout << "GQCS Error : ";
			//error_display(err_no);
			cout << endl;
			Disconnect(cl_id);
			if (overlap->type == IO_SEND)
				delete overlap;
			continue;
		}
		// IO 작업 유형을 결정하기 위해 완료 키를 확인
		switch (overlap->type)
		{
		case IO_RECV: {
			// 수신 완료 처리
			/*if (false == HandleReceive(cl_id, overlap, bytesTransferred))
				continue;*/
			//cout << "Received data: " << overlap->recvBuffer << endl;
			ClientInfo& cl = clients[cl_id];
			int remain_data = bytesTransferred + cl.prev;
			unsigned char* packet_start = overlap->recvBuffer;
			int packet_size = packet_start[0];

			while (packet_size <= remain_data) {
				process_packet(cl_id, packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0) packet_size = packet_start[0];
				else break;
			}

			if (0 < remain_data) {
				cl.prev = remain_data;
				memcpy(&overlap->recvBuffer, packet_start, remain_data);
			}
			cl.c_recv();
			break;
		}
		case IO_SEND:
			// 송신 완료 처리
			//HandleSend(overlap, bytesTransferred);
			if (bytesTransferred != overlap->wsabuf.len) {
				cout << "send 에러" << endl;
				Disconnect(cl_id);
			}
			delete overlap;
			break;
		case IO_ACCEPT: {
			//// Accept 완료 처리
			cout << "Accept Completed.\n";
			SOCKET c_socket = *(reinterpret_cast<SOCKET*>(overlap->recvBuffer));
			int a_id = get_id();
			if (-1 == a_id)
				cout << "over" << endl;
			else
			{
				ClientInfo& cl = clients[a_id];
				cl.cl_id = a_id;
				cl.cl_state = ST_ACCEPT;
				cl.prev = 0;
				cl.c_overlapped.type = IO_RECV;
				cl.c_overlapped.wsabuf.buf = reinterpret_cast<char*>(cl.c_overlapped.recvBuffer);
				cl.c_overlapped.wsabuf.len = sizeof(cl.c_overlapped.recvBuffer);
				ZeroMemory(&cl.c_overlapped.overlapped, sizeof(cl.c_overlapped.overlapped));
				cl.c_socket = c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), iocpHandle, a_id, 0);
				cl.c_recv();

			}
			ZeroMemory(&overlap->overlapped, sizeof(overlap->overlapped));
			c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			*(reinterpret_cast<SOCKET*>(overlap->recvBuffer)) = c_socket;
			AcceptEx(listensocket, c_socket, overlap->recvBuffer + 8, 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, NULL, &overlap->overlapped);
			//HandleAccept(overlap);
			break;
			// 필요에 따라 더 많은 경우를 추가 (예: IO_CONNECT)
		}
	/*	default:
			cout << "Unknown IO type" << endl;
			break;*/
		}
	}
}

bool lOCPServer::HandleAccept(Overlapped* overlapped)
{
	//// Accept 작업 완료 처리, 필요에 따라 새로운 워커 스레드 생성 또는 수락된 소켓 처리 등을 수행
	//// ...
	//// 새로운 Accept 작업을 등록하여 계속해서 들어오는 연결을 수신
	////PostAccept();
	DWORD dwBytes;
	cout << "Accept Completed.\n";
	SOCKET c_socket = *(reinterpret_cast<SOCKET*>(overlapped->recvBuffer));
	int a_id = get_id();
	
	ClientInfo& cl = clients[a_id];
	//cl.state_lock.lock();
	cl.cl_id = a_id;
	//cl.cl_state = ST_ACCEPT;
	//.cl.state_lock.unlock();
	cl.prev = 0;
	cl.c_overlapped.type = IO_RECV;
	cl.c_overlapped.wsabuf.buf = reinterpret_cast<char*>(cl.c_overlapped.recvBuffer);
	cl.c_overlapped.wsabuf.len = sizeof(cl.c_overlapped.recvBuffer);
	ZeroMemory(&cl.c_overlapped.overlapped, sizeof(cl.c_overlapped.overlapped));
	cl.c_socket = c_socket;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), iocpHandle, a_id, 0);
	cl.c_recv();
	

	ZeroMemory(&overlapped->overlapped, sizeof(overlapped->overlapped));
	c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	*(reinterpret_cast<SOCKET*>(overlapped->recvBuffer)) = c_socket;
	AcceptEx(listensocket, c_socket, overlapped->recvBuffer + 8, 0, sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16, &dwBytes, &overlapped->overlapped);
	return true;
	
}
void lOCPServer::Disconnect(int _s_id)
{
	cout << "서버 접속 종료";
	ClientInfo& cl = clients[_s_id];
	closesocket(clients[_s_id].c_socket);
}
bool lOCPServer::HandleReceive(int cl_id, Overlapped* overlapped, DWORD bytesTransferred)
{
	// 수신된 데이터 처리, 패킷 처리 등을 수행
	// overlapped->recvBuffer에서 수신된 데이터에 액세스할 수 있습니다.

	// 예시: 수신된 데이터 출력
	cout << "Received data: " << overlapped->recvBuffer << endl;
	ClientInfo& cl = clients[cl_id];
	int remain_data = bytesTransferred + cl.prev;
	unsigned char* packet_start = overlapped->recvBuffer;
	int packet_size = packet_start[0];

	while (packet_size <= remain_data) {
		process_packet(cl_id, packet_start);
		remain_data -= packet_size;
		packet_start += packet_size;
		if (remain_data > 0) packet_size = packet_start[0];
		else break;
	}

	if (0 < remain_data) {
		cl.prev = remain_data;
		memcpy(&overlapped->recvBuffer, packet_start, remain_data);
	}
	cl.c_recv();
	return true;
}

