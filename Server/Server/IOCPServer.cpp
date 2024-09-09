#include "pch.h"
#include "IOCPServer.h"

IOCPServer::IOCPServer() //: ready_count(0), ingamecount(0), gameRooms(4), clients(MAX_USER)
{
}

IOCPServer::~IOCPServer()
{
}

void IOCPServer::Initialize()
{
    WSAData WSAData;
    ::WSAStartup(MAKEWORD(2, 2), &WSAData);
    server_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

    SOCKADDR_IN server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(server_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    listen(server_socket, SOMAXCONN);

    std::cout << "서버 시작" << std::endl;

    g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(server_socket), g_h_iocp, 0, 0);

    g_timer = CreateEvent(NULL, FALSE, FALSE, NULL);

    // 클라이언트 초기화
    for (int i = 0; i < MAX_USER; ++i) { clients[i]._s_id = i;}
}

void IOCPServer::Run()
{
    std::vector<std::thread> worker_threads;
    //std::thread server_thread{ [&]() { /* Timer event handling function */ } };

    for (int i = 0; i < 16; ++i) {
        worker_threads.emplace_back([&]() { WorkerThread(); });
    }

    for (auto& th : worker_threads) {
        th.join();
    }
}

void IOCPServer::Shutdown()
{
    for (auto& cl : clients) {
        //if (ST_INGAME == cl._state)
            //Disconnect(cl._s_id);
    }

    closesocket(server_socket);
    WSACleanup();
}

void IOCPServer::WorkerThread()
{
}

void IOCPServer::HandleAccept(Overlap* exp_over)
{
}

void IOCPServer::HandleReceive(int _s_id, Overlap* exp_over, DWORD num_byte)
{
}

void IOCPServer::HandleSend(int _s_id, Overlap* exp_over, DWORD num_byte)
{
}
