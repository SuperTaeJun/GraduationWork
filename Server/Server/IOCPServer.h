#pragma once
#include "pch.h"

class PacketManager;
class IOCPServer
{
public:
    IOCPServer();
    virtual ~IOCPServer();
    void Initialize();
    void Run();


private:
    void WorkerThread();
    void HandleAccept(Overlap* exp_over);
    void HandleReceive(int _s_id, Overlap* exp_over, DWORD num_byte);
    void HandleSend(int _s_id, Overlap* exp_over, DWORD num_byte);
    void Disconnect(int _s_id);
    void ev_timer();

private:
    Overlap accept_ex;
    HANDLE g_h_iocp;
    HANDLE g_timer;
    SOCKET server_socket;
    vector<thread> worker_threads;
    std::unique_ptr<PacketManager> PM;
};

