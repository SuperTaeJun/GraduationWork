#include "pch.h"
#include "CorePch.h"
#include "lOCPServer.h"
int main()
{
    lOCPServer server;
    if (server.Init() && server.Start())
    {
        // 워커 스레드 생성
        server.CreateWorkerThreads();

        // 기타 작업 수행

        // 메인 스레드가 여기서 대기하거나 다른 작업을 수행할 수 있습니다.

        // 워커 스레드가 종료될 때까지 대기
        for (auto& thread : server.workerthread)
        {
            thread.join();
        }
    }
}