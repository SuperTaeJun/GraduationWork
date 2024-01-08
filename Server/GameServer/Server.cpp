#include "pch.h"
#include "CorePch.h"
#include "IOCPServer.h"



int main()
{
	IOCompletionPort iocp_server;
	if (iocp_server.Initialize()) {
		iocp_server.StartServer();
	}

}