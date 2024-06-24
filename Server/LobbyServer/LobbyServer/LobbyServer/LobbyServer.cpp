#include "LobbyServer.h"

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <map>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define LOBBY_PORT 12345
#define GAME_SERVER_IP "127.0.0.1"
#define GAME_SERVER_PORT 23456
#define BUFFER_SIZE 1024

struct ClientContext {
    SOCKET socket;
    OVERLAPPED overlapped;
    WSABUF buffer;
    char data[BUFFER_SIZE];
};

std::map<SOCKET, ClientContext*> clients;
HANDLE iocpHandle;

void InitializeIOCP() {
    iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (iocpHandle == NULL) {
        std::cerr << "CreateIoCompletionPort failed: " << GetLastError() << std::endl;
        exit(1);
    }
}

void AddSocketToIOCP(SOCKET socket) {
    ClientContext* context = new ClientContext();
    context->socket = socket;
    context->buffer.buf = context->data;
    context->buffer.len = BUFFER_SIZE;

    CreateIoCompletionPort((HANDLE)socket, iocpHandle, (ULONG_PTR)context, 0);
    clients[socket] = context;

    // Post initial read
    DWORD flags = 0;
    WSARecv(socket, &context->buffer, 1, NULL, &flags, &context->overlapped, NULL);
}

void AcceptConnections(SOCKET listenSocket) {
    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }
        std::cout << "Client connected: " << clientSocket << std::endl;
        AddSocketToIOCP(clientSocket);
    }
}

void TransferSocketToGameServer(SOCKET clientSocket) {
    SOCKET gameServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (gameServerSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create game server socket: " << WSAGetLastError() << std::endl;
        return;
    }

    sockaddr_in gameServerAddr;
   /* gameServerAddr.sin_family = AF_INET;
    gameServerAddr.sin_addr.s_addr = inet_addr(GAME_SERVER_IP);
    gameServerAddr.sin_port = htons(GAME_SERVER_PORT);*/

    if (connect(gameServerSocket, (sockaddr*)&gameServerAddr, sizeof(gameServerAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to game server: " << WSAGetLastError() << std::endl;
        closesocket(gameServerSocket);
        return;
    }

    WSAPROTOCOL_INFO protocolInfo;
    if (WSADuplicateSocket(clientSocket, GetCurrentProcessId(), &protocolInfo) == SOCKET_ERROR) {
        std::cerr << "Failed to duplicate socket: " << WSAGetLastError() << std::endl;
        closesocket(gameServerSocket);
        return;
    }

    send(gameServerSocket, (const char*)&protocolInfo, sizeof(protocolInfo), 0);

    // Notify the client to connect to the game server
    const char* message = "Transfer complete, connecting to game server...";
    send(clientSocket, message, strlen(message), 0);
    closesocket(clientSocket);

    // Close the game server socket after transferring
    closesocket(gameServerSocket);
}

void HandleIOCP() {
    while (true) {
        DWORD bytesTransferred;
        ULONG_PTR completionKey;
        LPOVERLAPPED overlapped;

        BOOL result = GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &completionKey, &overlapped, INFINITE);
        if (!result) {
            std::cerr << "GetQueuedCompletionStatus failed: " << GetLastError() << std::endl;
            continue;
        }

        ClientContext* context = (ClientContext*)completionKey;

        if (bytesTransferred == 0) {
            // Client disconnected
            std::cout << "Client disconnected: " << context->socket << std::endl;
            closesocket(context->socket);
            delete context;
            clients.erase(context->socket);
            continue;
        }

        // Process data
        std::cout << "Received data from client: " << context->data << std::endl;

        // Transfer the socket to the game server
        TransferSocketToGameServer(context->socket);

        // Remove client from IOCP after transferring
        closesocket(context->socket);
        delete context;
        clients.erase(context->socket);
    }
}

void StartLobbyServer() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(1);
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(LOBBY_PORT);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        exit(1);
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        exit(1);
    }

    InitializeIOCP();

    std::thread acceptThread(AcceptConnections, listenSocket);
    std::thread iocpThread(HandleIOCP);

    acceptThread.join();
    iocpThread.join();

    closesocket(listenSocket);
    WSACleanup();
}

int main() {
    StartLobbyServer();
    return 0;
}
