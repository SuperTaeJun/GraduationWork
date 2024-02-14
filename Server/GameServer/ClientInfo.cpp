#include "pch.h"
#include "ClientInfo.h"

ClientInfo::ClientInfo()
{
}


void ClientInfo::c_recv()
{
	DWORD recv_flag = 0;
	ZeroMemory(&c_overlapped.overlapped, sizeof(c_overlapped.overlapped));
	c_overlapped.wsabuf.buf = reinterpret_cast<char*>(c_overlapped.recvBuffer + prev);
	c_overlapped.wsabuf.len = sizeof(c_overlapped.recvBuffer) - prev;
	int ret = WSARecv(c_socket, &c_overlapped.wsabuf, 1, 0, &recv_flag, &c_overlapped.overlapped, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
			//error_display(error_num);
			WSAGetLastError();
	}
}

void ClientInfo::c_send(int num_bytes, void* mess)
{
	//int psize = reinterpret_cast<unsigned char*>(packet)[0];
	Overlapped* ex_over = new Overlapped(IO_SEND, num_bytes, mess);
	int ret = WSASend(c_socket, &ex_over->wsabuf, 1, 0, 0, &ex_over->overlapped, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
			WSAGetLastError();
	}
}
