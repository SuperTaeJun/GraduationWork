// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientSocket.h"
#include <sstream>
#include <process.h>
#include "Game/BOGameMode.h"
#include "Character/Character1.h"
#include "Character/CharacterBase.h"
#include "Player/CharacterController.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformAffinity.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"

ClientSocket::ClientSocket() :StopTaskCounter(0)
{
	
}

ClientSocket::~ClientSocket() {
	delete Thread;
	Thread = nullptr;

	closesocket(ServerSocket);
	WSACleanup();
}

bool ClientSocket::InitSocket()
{
	WSADATA wsaData;
	// 윈속 버전을 2.2로 초기화
	int nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nResult != 0) {
		UE_LOG(LogTemp, Error, TEXT("INITAILAIZING ERROR"));
		return false;
	}

	// TCP 소켓 생성
	ServerSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ServerSocket == INVALID_SOCKET) {
		return false;
	}
	return true;
}

bool ClientSocket::Connect(const char* s_IP, int port)
{
	// 접속할 서버 정보를 저장할 구조체
	SOCKADDR_IN stServerAddr;
	// 접속할 서버 포트 및 IP
	stServerAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, s_IP, &stServerAddr.sin_addr);
	stServerAddr.sin_port = htons(port);
	stServerAddr.sin_addr.s_addr = inet_addr(s_IP);

	int nResult = connect(ServerSocket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (nResult == SOCKET_ERROR) {
		return false;
	}
	return true;
}


void ClientSocket::CloseSocket()
{
	closesocket(ServerSocket);
	WSACleanup();
}

void ClientSocket::PacketProcess(unsigned char* ptr)
{
	//UE_LOG(LogClass, Warning, TEXT("init?"));
	//static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_OK: {
		SC_LOGIN_BACK* packet = reinterpret_cast<SC_LOGIN_BACK*>(ptr);
		//UE_LOG(LogClass, Warning, TEXT("RECV ROGIN?"));
		login_cond = true;
		CPlayer player;
		player.Id = packet->cl_id;
		player.X = packet->x;
		player.Y = packet->y;
		player.Z = packet->z;
		PlayerInfo.players[player.Id] = player;
		MyCharacterController->SetPlayerID(player.Id);
		MyCharacterController->SetPlayerInfo(&PlayerInfo);
		MyCharacterController->SetInitPlayerInfo(player);
		//UE_LOG(LogClass, Warning, TEXT("recv - id: %d, x: %d"), player.Id, player.X);
		break;
	}
	case SC_OTHER_PLAYER:
	{
		UE_LOG(LogClass, Warning, TEXT("other ROGIN?"));
		SC_PLAYER_SYNC* packet = reinterpret_cast<SC_PLAYER_SYNC*>(ptr);
		auto info = make_shared<CPlayer>();
		info->Id = packet->id;
		info->X = packet->x;
		info->Y = packet->y;
		info->Z = packet->z;
		info->Yaw = packet->yaw;
		//float z = packet->z;
		//UE_LOG(LogClass, Warning, TEXT("recv data"));
		MyCharacterController->SetNewCharacterInfo(info);
		break;
	}
	case SC_MOVE_PLAYER:
	{
		//UE_LOG(LogClass, Warning, TEXT("recv move?"));
		CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(ptr);
		PlayerInfo.players[packet->id].X = packet->x;
		PlayerInfo.players[packet->id].Y = packet->y;
		PlayerInfo.players[packet->id].Z = packet->z;
		PlayerInfo.players[packet->id].Yaw = packet->yaw;
		PlayerInfo.players[packet->id].VeloX = packet->vx;
		PlayerInfo.players[packet->id].VeloY = packet->vy;
		PlayerInfo.players[packet->id].VeloZ = packet->vz;
		PlayerInfo.players[packet->id].Max_Speed = packet->Max_speed;

		break;
	}
	case SC_CHAR_BACK: {
		break;
	}
	case SC_WEP_BACK: {
		break;
	}
	default:
		break;
	}
}

void ClientSocket::Send_Login_Info(char* id, char* pw)
{
	//패킷 조립
	CS_LOGIN_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_LOGIN;
	strcpy(packet.id, _id);
	strcpy(packet.pw, _pw);

	auto player= Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(MyCharacterController, 0));
	//cs_login_packet
	auto location = player->GetActorLocation();
	packet.z = location.Z;
	SendPacket(&packet);
	//UE_LOG(LogClass, Warning, TEXT("Sending login info - id: %s, pw: %s"), ANSI_TO_TCHAR(id), ANSI_TO_TCHAR(pw));

}

void ClientSocket::Send_Move_Packet(int sessionID, FVector Location, FRotator Rotation, FVector Velocity, float Max_speed)
{
	if (login_cond) {
		CS_MOVE_PACKET packet;
		packet.size = sizeof(packet);
		packet.type = CS_MOVE;
		packet.id = sessionID;
		packet.x = Location.X;
		packet.y = Location.Y;
		packet.z = Location.Z;
		packet.yaw = Rotation.Yaw;
		packet.vx = Velocity.X;
		packet.vy = Velocity.Y;
		packet.vz = Velocity.Z;
		packet.Max_speed = Max_speed;
		SendPacket(&packet);
		//UE_LOG(LogClass, Warning, TEXT("send move"));
	}
}

void ClientSocket::Send_Character_Type(PlayerType type)
{
	CS_SELECT_CHARACTER packet;
	packet.size = sizeof(packet);
	packet.type = CS_SELECT_CHAR;
	packet.character_type = type;
	SendPacket(&packet);
}

void ClientSocket::Send_Weapon_Type(WeaponType type, int id)
{
	CS_SELECT_WEAPO packet;
	packet.size = sizeof(packet);
	packet.type = CS_SELECT_WEP;
	packet.id = id;
	packet.weapon_type = type;
	SendPacket(&packet);
}

bool ClientSocket::Init()
{
	return true;
}
uint32 ClientSocket::Run()
{
	FPlatformProcess::Sleep(0.03);
	//Connect();
	//Send_Login_Info();

	FPlatformProcess::Sleep(0.03);
	while (StopTaskCounter.GetValue() == 0 && MyCharacterController != nullptr)
	{
		// 블로킹 소켓을 이용하여 패킷 수신
		unsigned char buffer[10000];
		int receivedBytes = recv(ServerSocket, reinterpret_cast<char*>(buffer), 10000, 0);

		if (receivedBytes > 0)
		{
			// 패킷 처리
			PacketProcess(buffer);
		}
		else if (receivedBytes == 0)
		{
			// 연결이 끊긴 경우 처리
			// 예: closesocket(ServerSocket);
			break;
		}
		else
		{
			// 오류 처리
			int errorNumber = WSAGetLastError();
			// 예: handle error
			break;
		}
	}

	return 0;
}

void ClientSocket::Stop()
{
	// thread safety 변수를 조작해 while loop 가 돌지 못하게 함
	StopTaskCounter.Increment();
}

void ClientSocket::Exit()
{
}

bool ClientSocket::StartListen()
{
	// 스레드 시작
	if (Thread != nullptr) return false;
	Thread = FRunnableThread::Create(this, TEXT("ClientSocket"), 0, TPri_BelowNormal);
	return (Thread != nullptr);
}

void ClientSocket::StopListen()
{
	// 스레드 종료
	Stop();
	Thread->WaitForCompletion();
	Thread->Kill();
	delete Thread;
	Thread = nullptr;
	StopTaskCounter.Reset();
}

void ClientSocket::RecvPacket()
{
	//UE_LOG(LogClass, Warning, TEXT("recv data"));
	DWORD recv_flag = 0;
	ZeroMemory(&_recv_over.overlapped, sizeof(_recv_over.overlapped));
	_recv_over.wsabuf.buf = reinterpret_cast<char*>(_recv_over.recvBuffer + _prev_size);
	_recv_over.wsabuf.len = sizeof(_recv_over.recvBuffer) - _prev_size;
	int ret = WSARecv(ServerSocket, &_recv_over.wsabuf, 1, 0, &recv_flag, &_recv_over.overlapped, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
	}
	/*if (ret > 0) {
		UE_LOG(LogClass, Warning, TEXT("recv 됨 "));
	}*/
}

void ClientSocket::SendPacket(void* packet)
{
	//UE_LOG(LogClass, Warning, TEXT("send data"));
	int psize = reinterpret_cast<unsigned char*>(packet)[0];
	Overlapped* ex_over = new Overlapped(IO_SEND, psize, packet);
	int ret = WSASend(ServerSocket, &ex_over->wsabuf, 1, 0, 0, &ex_over->overlapped, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
			WSAGetLastError();
	}
}

void ClientSocket::SetPlayerController(ACharacterController* CharacterController)
{
	if (CharacterController)
	{
		MyCharacterController = CharacterController;
	}
}
