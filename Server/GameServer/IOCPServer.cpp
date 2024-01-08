#include "pch.h"
#include "IOCPServer.h"


unsigned int WINAPI CallWorkerThread(LPVOID p) {
	IOCompletionPort* pOverlappedEvent = (IOCompletionPort*)p;
	pOverlappedEvent->WorkerThread();
	return 0;
}

IOCompletionPort::IOCompletionPort() {
	bWorkerThread = true;
	bAccept = true;

}

IOCompletionPort::~IOCompletionPort() {
	// winsock 의 사용을 끝낸다
	WSACleanup();
	// 다 사용한 객체를 삭제
	if (pSocketInfo) {
		delete[] pSocketInfo;
		pSocketInfo = NULL;
	}
	if (hWorkerHandle) {
		delete[] hWorkerHandle;
		hWorkerHandle = NULL;
	}
}

bool IOCompletionPort::Initialize() {
	WSADATA wsaData;
	int nResult;
	// winsock 2.2 버전으로 초기화
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0) {
		cout << "winsock 초기화 실패" << endl;
		return false;
	}

	// 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET) {
		cout << "소켓 생성 실패" << endl;
		return false;
	}

	// 서버 정보 설정
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(7777);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 소켓 설정
	nResult = bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	if (nResult == SOCKET_ERROR) {
		cout <<"bind 실패" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// 수신 대기열 생성
	nResult = listen(listenSocket, 5);
	if (nResult == SOCKET_ERROR) {
		cout << "listen 실패" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}
	return true;
}

void IOCompletionPort::StartServer() {
	int nResult;
	// 클라이언트 정보
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket;
	DWORD recvBytes;
	DWORD flags;

	// Completion Port 객체 생성
	hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	// Worker Thread 생성
	if (!CreateWorkerThread()) return;

	cout << "서버 시작" << endl;

	// 클라이언트 접속을 받음
	while (bAccept) {
		clientSocket = WSAAccept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen, NULL, NULL);

		if (clientSocket == INVALID_SOCKET) {
			cout << "accept 실패" << endl;
			return;
		}

		pSocketInfo = new stSOCKETINFO();
		pSocketInfo->socket = clientSocket;
		pSocketInfo->recvBytes = 0;
		pSocketInfo->sendBytes = 0;
		pSocketInfo->dataBuf.len = 1024;
		pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
		flags = 0;

		hIOCP = CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (DWORD)pSocketInfo, 0);

		// 중첩 소켓을 지정하고 완료시 실행될 함수를 넘겨줌
		nResult = WSARecv(
			pSocketInfo->socket,
			&pSocketInfo->dataBuf,
			1,
			&recvBytes,
			&flags,
			&(pSocketInfo->overlapped),
			NULL
		);

		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
			cout << "[ERROR] IO Pending 실패 : " << WSAGetLastError() << endl;
			return;
		}
	}
}

bool IOCompletionPort::CreateWorkerThread() {
	unsigned int threaddId;
	// 시스템 정보 가져옴
	unsigned int numCores = std::thread::hardware_concurrency();
	cout <<"CPU 갯수 : " << numCores << endl;
	int nThreadCnt = numCores * 2;

	// thread handler 선언
	hWorkerHandle = new HANDLE[nThreadCnt];
	// thread 생성
	for (int i = 0; i < nThreadCnt; i++) {
		hWorkerHandle[i] = (HANDLE*)_beginthreadex(NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId);
		if (hWorkerHandle[i] == NULL) {
			cout << "생성 실패" << endl;
			return false;
		}
		ResumeThread(hWorkerHandle[i]);
	}
	cout << "시작" << endl;
	cout << endl;
	return true;
}

void IOCompletionPort::WorkerThread() {
	// 함수 호출 성공 여부
	BOOL    bResult;
	int     nResult;
	// Overlapped I/O 작업에서 전송된 데이터 크기
	DWORD   recvBytes;
	DWORD   sendBytes;
	// Completion Key를 받을 포인터 변수
	stSOCKETINFO* pCompletionKey;
	// I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터    
	stSOCKETINFO* pSocketInfo;
	// 
	DWORD   dwFlags = 0;

	while (bWorkerThread) {
		/*
		* 이 함수로 인해 쓰레드들은 WaitingThread Queue 에 대기상태로 들어가게 됨
		* 완료된 Overlapped I/O 작업이 발생하면 IOCP Queue 에서 완료된 작업을 가져와
		* 뒷처리를 함
		*/
		bResult = GetQueuedCompletionStatus(hIOCP,
			&recvBytes,                // 실제로 전송된 바이트
			(PULONG_PTR)&pCompletionKey,    // completion key
			(LPOVERLAPPED*)&pSocketInfo,            // overlapped I/O 객체
			INFINITE                // 대기할 시간
		);

		if (recvBytes > 0) {
			cout << "[INFO] 소켓(" << pSocketInfo->socket << ")로부터 데이터 수신: " << pSocketInfo->dataBuf.buf << endl;

			// todo

			// 추가 데이터를 수신하기 위해 다시 WSARecv 작업을 시작
			pSocketInfo->recvBytes = 0;
			DWORD flags = 0;
			nResult = WSARecv(
				pSocketInfo->socket,
				&pSocketInfo->dataBuf,
				1,
				&recvBytes,
				&flags,
				&(pSocketInfo->overlapped),
				NULL
			);

			if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
				cout << "[ERROR] WSARecv 실패: " << WSAGetLastError() << endl;
				closesocket(pSocketInfo->socket);
				free(pSocketInfo);
				continue;
			}
		}
		else {
			// 0 바이트를 받음 - 연결이 닫힐 수 있으므로 이에 대한 처리를 수행
			cout << "[INFO] 소켓(" << pSocketInfo->socket << ")로부터 0 바이트 수신" << endl;
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
		}
		// todo
	}
}

