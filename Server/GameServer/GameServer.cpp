#include "pch.h"
#include "CorePch.h"
#include "lOCPServer.h"
int main()
{
    wcout.imbue(locale("korean"));
    
    lOCPServer server;
    if (server.Init())
    {
        server.Start();
        // 워커 스레드 생성
        for (auto& thread : server.workerthread)
        {
            thread.join();
        }
    }

}