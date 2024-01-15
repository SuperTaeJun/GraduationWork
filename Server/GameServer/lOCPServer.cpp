#include "pch.h"
#include "lOCPServer.h"

lOCPServer::lOCPServer()
{
}

lOCPServer::~lOCPServer()
{
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
	serverAddr.sin_port = ::htons(7777);
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
	overlappedData->type = IO_RECV;
	DWORD bytesReceived;
	if (!AcceptEx(listensocket, acceptSocket, overlappedData->messageBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytesReceived, &(overlappedData->overlapped)))
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			// 俊矾 贸府
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

	// 弥家 1俺狼 况目 胶饭靛甫 积己
	int WorkerCount = numCores * 2;
	// 况目 胶饭靛 积己
	for (int i = 0; i < WorkerCount; ++i)
		workerthread.emplace_back([this]() {WorkerThread(); });

	std::cout << "Created " << WorkerCount << " worker threads." << std::endl;
}

void lOCPServer::WorkerThread()
{
	while (true)
	{

	}
}
