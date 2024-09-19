#include "pch.h"
#include "IOCPServer.h"

IOCPServer::IOCPServer() //: ready_count(0), ingamecount(0), gameRooms(4), clients(MAX_USER)
{
   /* ready_count = 0;
    ingamecount = 0;*/
}

IOCPServer::~IOCPServer()
{
	CloseHandle(g_h_iocp);
	CloseHandle(g_timer); 
	closesocket(server_socket);
}

void IOCPServer::Initialize()
{
    WSAData WSAData;
    ::WSAStartup(MAKEWORD(2, 2), &WSAData);
    server_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

    DWORD dwBytes;
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

    SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    char   accept_buf[sizeof(SOCKADDR_IN) * 2 + 32 + 100];
    //Overlap   accept_ex;
    *(reinterpret_cast<SOCKET*>(&accept_ex._net_buf)) = c_socket;
    ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
    accept_ex._op = IO_ACCEPT;

    AcceptEx(server_socket, c_socket, accept_buf, 0, sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);

    // 클라이언트 초기화
    for (int i = 0; i < MAX_USER; ++i) { clients[i]._s_id = i;}
}



void IOCPServer::Run()
{
  
    thread timer_thread([this]() { ev_timer(); });
    for (int i = 0; i < 16; ++i) {
        worker_threads.emplace_back([&]() { WorkerThread(); });
    }

    for (auto& th : worker_threads) {
        th.join();
    }
    if (timer_thread.joinable())
        timer_thread.join();
}



void IOCPServer::WorkerThread()
{
	while (1) {
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int _s_id = static_cast<int>(iocp_key);
		Overlap* exp_over = reinterpret_cast<Overlap*>(p_over);
		if (FALSE == ret) {
			int err_no = WSAGetLastError();
			cout << "GQCS Error : ";
			error_display(err_no);
			cout << endl;
			Disconnect(_s_id);
			if (exp_over->_op == IO_SEND)
				delete exp_over;
			continue;
		}

		switch (exp_over->_op) {
		case IO_RECV: {
			if (num_byte == 0) {
				cout << "연결종료" << endl;
				Disconnect(_s_id);
				continue;
			}
			CLIENT& cl = clients[_s_id];
			int remain_data = num_byte + cl._prev_size;
			unsigned char* packet_start = exp_over->_net_buf;
			int packet_size = packet_start[0];

			while (packet_size <= remain_data) {
				PM->ProcessPacket(_s_id, packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0) packet_size = packet_start[0];
				else break;
			}

			if (0 < remain_data) {
				cl._prev_size = remain_data;
				memcpy(&exp_over->_net_buf, packet_start, remain_data);
			}
			if (remain_data == 0)
				cl._prev_size = 0;
			cl.do_recv();
			break;
		}
		case IO_SEND: {
			if (num_byte != exp_over->_wsa_buf.len) {
				cout << "send 에러" << endl;
				Disconnect(_s_id);
			}
			delete exp_over;
			break;
		}
		case IO_ACCEPT: {
			cout << "Accept Completed.\n";
			SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->_net_buf));

			int n__s_id = PM->get_id();
			cout << "n__s_id : " << n__s_id << endl;
			if (-1 == n__s_id) {
				cout << "user over.\n";
			}
			else {
				CLIENT& cl = clients[n__s_id];
				cl.state_lock.lock();
				cl._s_id = n__s_id;
				cl._state = ST_ACCEPT;
				cl.state_lock.unlock();
				cl._prev_size = 0;
				cl._recv_over._op = IO_RECV;
				cl._recv_over._wsa_buf.buf = reinterpret_cast<char*>(cl._recv_over._net_buf);
				cl._recv_over._wsa_buf.len = sizeof(cl._recv_over._net_buf);
				ZeroMemory(&cl._recv_over._wsa_over, sizeof(cl._recv_over._wsa_over));
				cl._socket = c_socket;

				CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, n__s_id, 0);
				cl.do_recv();
			}

			ZeroMemory(&exp_over->_wsa_over, sizeof(exp_over->_wsa_over));
			c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			*(reinterpret_cast<SOCKET*>(exp_over->_net_buf)) = c_socket;
			AcceptEx(server_socket, c_socket, exp_over->_net_buf + 8, 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, NULL, &exp_over->_wsa_over);


			break;
		}
		}
	}
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

void IOCPServer::Disconnect(int _s_id)
{
    CLIENT& cl = clients[_s_id];
    clients[_s_id].state_lock.lock();
    clients[_s_id]._state = ST_FREE;
    clients[_s_id].state_lock.unlock();
    closesocket(clients[_s_id]._socket);
    cout << "------------연결 종료------------" << endl;
}

void IOCPServer::ev_timer()
{
   /* WaitForSingleObject(g_timer, INFINITE);
    {
        timer_q.Clear();
    }
    while (true) {
        timer_ev order;
        timer_q.WaitPop(order);
        auto t = order.start_t - chrono::system_clock::now();
        int s_id = order.this_id;
        if (clients[s_id]._state != ST_INGAME) continue;
        if (clients[s_id]._is_active == false) continue;
        if (order.start_t <= chrono::system_clock::now()) {
        }
        else {
            timer_q.Push(order);
            this_thread::sleep_for(10ms);
        }
    }*/
}
