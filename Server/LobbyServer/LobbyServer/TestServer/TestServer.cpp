#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <map>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

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

void ReceiveTransferredSocket() {
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create listen socket: " << WSAGetLastError() << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(GAME_SERVER_PORT);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return;
    }

    while (true) {
        SOCKET transferSocket = accept(listenSocket, NULL, NULL);
        if (transferSocket == INVALID_SOCKET) {
            std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        WSAPROTOCOL_INFO protocolInfo;
        int receivedBytes = recv(transferSocket, (char*)&protocolInfo, sizeof(protocolInfo), 0);
        if (receivedBytes != sizeof(protocolInfo)) {
            std::cerr << "Failed to receive protocol info" << std::endl;
            closesocket(transferSocket);
            continue;
        }

        SOCKET clientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, &protocolInfo, 0, WSA_FLAG_OVERLAPPED);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "WSASocket failed: " << WSAGetLastError() << std::endl;
            closesocket(transferSocket);
            continue;
        }

        std::cout << "Received client socket from lobby server" << std::endl;
        AddSocketToIOCP(clientSocket);
        closesocket(transferSocket);
    }

    closesocket(listenSocket);
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

        // Process game data
        std::cout << "Received game data from client: " << context->data << std::endl;

        // Post another read
        DWORD flags = 0;
        WSARecv(context->socket, &context->buffer, 1, NULL, &flags, &context->overlapped, NULL);
    }
}

void StartGameServer() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    InitializeIOCP();

    std::thread receiveThread(ReceiveTransferredSocket);
    std::thread iocpThread(HandleIOCP);

    receiveThread.join();
    iocpThread.join();

    WSACleanup();
}

int main() {
    StartGameServer();
    return 0;
}
