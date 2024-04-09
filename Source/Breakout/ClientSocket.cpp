// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientSocket.h"
#include <sstream>
#include <process.h>
#include "Game/BOGameInstance.h"
#include "Game/BOGameMode.h"
#include "Character/Character1.h"
#include "Character/CharacterBase.h"
#include "Player/CharacterController.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformAffinity.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#pragma region Main Thread Code
ClientSocket::ClientSocket() :StopTaskCounter(0)
{
	/*gameinst = inst;*/
	
	
	
}

ClientSocket::~ClientSocket() {
	if (Thread)
	{
		// 스레드 종료
		Thread->WaitForCompletion();
		Thread->Kill();
		delete Thread;
	}
}

bool ClientSocket::InitSocket()
{

	return true;
}

bool ClientSocket::Connect()
{

	WSADATA wsaData;
	// 윈속 버전을 2.2로 초기화
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("INITAILAIZING ERROR"));
		return false;
	}

	// TCP 소켓 생성
	ServerSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ServerSocket == INVALID_SOCKET) {
		return false;
	}

	// 접속할 서버 정보를 저장할 구조체
	SOCKADDR_IN stServerAddr;
	::memset(&stServerAddr, 0, sizeof(stServerAddr));
	// 접속할 서버 포트 및 IP
	stServerAddr.sin_family = AF_INET;
	::inet_pton(AF_INET, SERVER_IP, &stServerAddr.sin_addr);
	stServerAddr.sin_port = htons(SERVER_PORT);
	//stServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	int nResult = connect(ServerSocket, (sockaddr*)&stServerAddr, sizeof(stServerAddr));
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


bool ClientSocket::PacketProcess(char* ptr)
{
	//UE_LOG(LogClass, Warning, TEXT("init?"));
	//static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_OK: {
		SC_LOGIN_BACK* packet = reinterpret_cast<SC_LOGIN_BACK*>(ptr);
		//to_do
		gameinst->SetPlayerID(packet->id);
		UE_LOG(LogClass, Warning, TEXT("aaaaa"));
	
		break;
	}
	case SC_OTHER_PLAYER:
	{
		//UE_LOG(LogClass, Warning, TEXT("other ROGIN?"));
		SC_PLAYER_SYNC* packet = reinterpret_cast<SC_PLAYER_SYNC*>(ptr);
		auto info = make_shared<CPlayer>();
		info->Id = packet->id;
		info->X = packet->x;
		info->Y = packet->y;
		info->Z = packet->z;
		info->Yaw = packet->yaw;
		info->p_type = packet->p_type;
		//float z = packet->z;
		UE_LOG(LogClass, Warning, TEXT("recv - info->id: %d,"), info->Id);
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

		UE_LOG(LogClass, Warning, TEXT("recv - move player id : %d,"), packet->id);
		break;
	}
	case SC_CHAR_BACK: {
		SC_SELECT_CHARACTER_BACK* packet = reinterpret_cast<SC_SELECT_CHARACTER_BACK*>(ptr);
		CPlayer player;
		player.Id = packet->clientid;
		player.X = packet->x;
		player.Y = packet->y;
		player.Z = packet->z;
		player.p_type = packet->p_type;
		PlayerInfo.players[player.Id] = player;
		//MyCharacterController->SetPlayerID(player.Id);
		MyCharacterController->SetPlayerInfo(&PlayerInfo);
		MyCharacterController->SetInitPlayerInfo(player);

		//CS_SIGNAL_PACKET repacket;
		//repacket.size = sizeof(repacket);
		//repacket.type = CS_SIGNAl;
		//SendPacket(&repacket);
		break;
	}
	case SC_OTHER_WEAPO: {
		SC_SYNC_WEAPO* packet = reinterpret_cast<SC_SYNC_WEAPO*>(ptr);
		PlayerInfo.players[packet->id].w_type = packet->weapon_type;
		//float z = packet->z;
		//UE_LOG(LogClass, Warning, TEXT("recv data"));

		break;
	}
	case SC_ALL_READY: {
		SC_ACCEPT_READY* packet = reinterpret_cast<SC_ACCEPT_READY*>(ptr);
		UE_LOG(LogTemp, Warning, TEXT("recv - all ready packet"));
		bAllReady = true;
		break;
	}
	// 공격 나이아가라 이팩트 효과
	case SC_ATTACK: {
		UE_LOG(LogTemp, Warning, TEXT("chong"));
		SC_ATTACK_PLAYER* packet = reinterpret_cast<SC_ATTACK_PLAYER*>(ptr);
		PlayerInfo.players[packet->clientid].Sshot.X = packet->sx;
		PlayerInfo.players[packet->clientid].Sshot.Y = packet->sy;
		PlayerInfo.players[packet->clientid].Sshot.Z = packet->sz;
		PlayerInfo.players[packet->clientid].Eshot.X = packet->ex;
		PlayerInfo.players[packet->clientid].Eshot.Y = packet->ey;
		PlayerInfo.players[packet->clientid].Eshot.Z = packet->ez;
		//UE_LOG(LogTemp, Warning, TEXT("%f, %f"), packet->sx, packet->ex);
		MyCharacterController->SetAttack(packet->clientid);
		// = packet->hp;1
		//PlayerInfo.players[packet].w_type = packet->weapon_type;	}
		break;
	}
	//이팩트 처리
	case SC_EFFECT: {
		CS_EFFECT_PACKET* packet = reinterpret_cast<CS_EFFECT_PACKET*>(ptr);
		PlayerInfo.players[packet->attack_id].Hshot.X = packet->lx;
		PlayerInfo.players[packet->attack_id].Hshot.Y = packet->ly;
		PlayerInfo.players[packet->attack_id].Hshot.Z = packet->lz;
		PlayerInfo.players[packet->attack_id].FEffect.Pitch = packet->r_pitch;
		PlayerInfo.players[packet->attack_id].FEffect.Yaw = packet->r_yaw;
		PlayerInfo.players[packet->attack_id].FEffect.Roll = packet->r_roll;
		//UE_LOG(LogTemp, Warning, TEXT("%f, %f"), packet->sx, packet->ex);
		MyCharacterController->SetHitEffect(packet->attack_id);
		break;
	}
	//HP동기화 처리
	case SC_PLAYER_DAMAGE: {
		SC_DAMAGE_CHANGE* packet = reinterpret_cast<SC_DAMAGE_CHANGE*>(ptr);
		CPlayer player;
		player.Id = packet->damaged_id;
		player.hp = packet->hp;
		
		MyCharacterController->SetHp(player.hp);
		break;
	}
	default:
		break;
	}
	return true;
}

void ClientSocket::Send_Login_Info(char* id, char* pw)
{
	//패킷 조립
	CS_LOGIN_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_LOGIN;
	strcpy_s(packet.id, id);
	strcpy_s(packet.pw, pw);
	//cs_login_packet
	
	SendPacket(&packet);
	//Send(packet.size, &packet);
	UE_LOG(LogClass, Warning, TEXT("Sending login info - id: %s, pw: %s"), ANSI_TO_TCHAR(id), ANSI_TO_TCHAR(pw));
	
}

void ClientSocket::Send_Move_Packet(int sessionID, FVector Location, FRotator Rotation, FVector Velocity, float Max_speed)
{
	//if (login_cond == true) {
		CS_MOVE_PACKET packet;
		packet.size = sizeof(packet);
		packet.type = CS_MOVE_Packet;
		packet.id = sessionID;
		packet.x = Location.X;
		packet.y = Location.Y;
		packet.z = Location.Z;
		packet.yaw = Rotation.Yaw;
		packet.vx = Velocity.X;
		packet.vy = Velocity.Y;
		packet.vz = Velocity.Z;
		packet.Max_speed = Max_speed;
		//Send(packet.size, &packet);
		SendPacket(&packet);
		//UE_LOG(LogClass, Warning, TEXT("send move"));
	//}
}

void ClientSocket::Send_Character_Type(PlayerType type, int id)
{
	auto player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(MyCharacterController, 0));
	CS_SELECT_CHARACTER packet;
	packet.size = sizeof(packet);
	packet.type = CS_SELECT_CHAR;
	packet.id = id;
	//Send(packet.size, &packet);
	auto location = player->GetActorLocation();
	packet.x = location.X;
	packet.y = location.Y;
	packet.z = location.Z;
	//packet.p_type = character_type;
	packet.p_type = type;
	SendPacket(&packet);
}

void ClientSocket::Send_Weapon_Type(WeaponType type, int sessionID)
{
	CS_SELECT_WEAPO packet;
	packet.size = sizeof(packet);
	packet.type = CS_SELECT_WEP;
	packet.id = sessionID;
	packet.weapon_type = type;
	//Send(packet.size, &packet);
	SendPacket(&packet);
}
void ClientSocket::Send_Ready_Packet(bool ready)
{
	CS_READY_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_READY;
	SendPacket(&packet);
}
void ClientSocket::Send_Fire_Effect(int attack_id, FVector ImLoc, FRotator ImRot)
{
	CS_EFFECT_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_HIT_EFFECT;
	packet.attack_id = attack_id;
	packet.lx = ImLoc.X;
	packet.ly = ImLoc.Y;
	packet.lz = ImLoc.Z;
	packet.r_pitch = ImRot.Pitch;
	packet.r_yaw = ImRot.Yaw;
	packet.r_roll = ImRot.Roll;
	SendPacket(&packet);
}
void ClientSocket::Send_AttackPacket(int attack_id, FVector SLoc, FVector ELoc)
{
	UE_LOG(LogClass, Warning, TEXT("Send_AttackPacket"));
	CS_ATTACK_PLAYER packet;
	packet.size = sizeof(packet);
	packet.type = CS_ATTACK;
	packet.attack_id = attack_id;
	packet.sx = SLoc.X;
	packet.sy = SLoc.Y;
	packet.sz = SLoc.Z;
	packet.ex = ELoc.X;
	packet.ey = ELoc.Y;
	packet.ez = ELoc.Z;
	SendPacket(&packet);
}
void ClientSocket::Send_Damage_Packet(int damaged_id, float damage)
{
	CS_DAMAGE_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_DAMAGE;
	packet.damaged_id = damaged_id;
	packet.damage = damage;
	SendPacket(&packet);
}
bool ClientSocket::Init()
{
	//UE_LOG(LogTemp, Warning, TEXT("Thread has been initialized"));
	return true;
}
uint32 ClientSocket::Run()
{
	// 언리얼 엔진 로그 출력
	FPlatformProcess::Sleep(0.03);
	//	Concurrency::concurrent_queue<char> buffer;
		////Connect();
	Iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(ServerSocket), Iocp, 0, 0);

	RecvPacket();

	//Send_LoginPacket();

	SleepEx(0, true);
	//StopTaskCounter.GetValue() == 0
	// recv while loop 시작
	// StopTaskCounter 클래스 변수를 사용해 Thread Safety하게 해줌
	while (StopTaskCounter.GetValue() == 0)
	{
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over;

		BOOL ret = GetQueuedCompletionStatus(Iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		Overlap* exp_over = reinterpret_cast<Overlap*>(p_over);

		if (false == ret) {
			int err_no = WSAGetLastError();
			if (exp_over->_op == IO_SEND)
				delete exp_over;
			continue;
		}

		switch (exp_over->_op) {
		case IO_RECV: {
			if (num_byte == 0) {
				//Disconnect();
				continue;
			}
			int remain_data = num_byte + _prev_size;
			char* packet_start = exp_over->_net_buf;
			int packet_size = packet_start[0];
			while (packet_size <= remain_data) {
				PacketProcess(packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0) packet_size = packet_start[0];
				else break;
			}

			if (0 < remain_data) {
				_prev_size = remain_data;
				memcpy(&exp_over->_net_buf, packet_start, remain_data);
			}

			RecvPacket();
			SleepEx(0, true);
			break;
		}
		case IO_SEND: {
			if (num_byte != exp_over->_wsa_buf.len) {
				//Disconnect();
			}
			delete exp_over;
			break;
		}

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
	if (ServerSocket)
	{
		closesocket(ServerSocket);
		WSACleanup();
	}
}

bool ClientSocket::StartListen()
{
	// 스레드 시작
	if (Thread != nullptr) return false;
	Thread = FRunnableThread::Create(this, TEXT("ClientSocket"), 0, TPri_BelowNormal);
	return (Thread != nullptr);
}
//
//void ClientSocket::StopListen()
//{
//	// 스레드 종료
//	Stop();
//	Thread->WaitForCompletion();
//	Thread->Kill();
//	delete Thread;
//	Thread = nullptr;
//	StopTaskCounter.Reset();
//}

void ClientSocket::SetPlayerController(ACharacterController* CharacterController)
{
	if (CharacterController)
	{
		MyCharacterController = CharacterController;
	}
}


void ClientSocket::RecvPacket()
{
	//UE_LOG(LogClass, Warning, TEXT("recv data"));
	DWORD recv_flag = 0;
	ZeroMemory(&_recv_over._wsa_over, sizeof(_recv_over._wsa_over));
	_recv_over._wsa_buf.buf = reinterpret_cast<char*>(_recv_over._net_buf + _prev_size);
	_recv_over._wsa_buf.len = sizeof(_recv_over._net_buf) - _prev_size;
	int ret = WSARecv(ServerSocket, &_recv_over._wsa_buf, 1, 0, &recv_flag, &_recv_over._wsa_over, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
	}

}
void ClientSocket::SendPacket(void* packet)
{
	int psize = reinterpret_cast<unsigned char*>(packet)[0];
	Overlap* ex_over = new Overlap(IO_SEND, psize, packet);
	int ret = WSASend(ServerSocket, &ex_over->_wsa_buf, 1, 0, 0, &ex_over->_wsa_over, NULL);
	if (SOCKET_ERROR == ret) {
		int error_num = WSAGetLastError();
		if (ERROR_IO_PENDING != error_num)
			WSAGetLastError();
	}
}