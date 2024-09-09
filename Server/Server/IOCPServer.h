#pragma once
#include "pch.h"

class IOCPServer
{
public:
    IOCPServer();
    ~IOCPServer();
    void Initialize();
    void Run();
    void Shutdown();

private:
    void WorkerThread();
    void HandleAccept(Overlap* exp_over);
    void HandleReceive(int _s_id, Overlap* exp_over, DWORD num_byte);
    void HandleSend(int _s_id, Overlap* exp_over, DWORD num_byte);

    HANDLE g_h_iocp;
    HANDLE g_timer;
    SOCKET server_socket;
};

