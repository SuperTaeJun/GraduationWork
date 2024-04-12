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
	case SC_SHOTGUN_BEAM: {
		CS_SHOTGUN_BEAM_PACKET* packet = reinterpret_cast<CS_SHOTGUN_BEAM_PACKET*>(ptr);
		PlayerInfo.players[packet->attackid].sSshot.X = packet->sx;
		PlayerInfo.players[packet->attackid].sSshot.Y = packet->sy;
		PlayerInfo.players[packet->attackid].sSshot.Z = packet->sz;
		PlayerInfo.players[packet->attackid].sEshot.X = packet->ex0;
		PlayerInfo.players[packet->attackid].sEshot.Y = packet->ey0;
		PlayerInfo.players[packet->attackid].sEshot.Z = packet->ez0;
		PlayerInfo.players[packet->attackid].sEshot1.X = packet->ex1;
		PlayerInfo.players[packet->attackid].sEshot1.Y = packet->ey1;
		PlayerInfo.players[packet->attackid].sEshot1.Z = packet->ez1;
		PlayerInfo.players[packet->attackid].sEshot2.X = packet->ex2;
		PlayerInfo.players[packet->attackid].sEshot2.Y = packet->ey2;
		PlayerInfo.players[packet->attackid].sEshot2.Z = packet->ez2;
		PlayerInfo.players[packet->attackid].sEshot3.X = packet->ex3;
		PlayerInfo.players[packet->attackid].sEshot3.Y = packet->ey3;
		PlayerInfo.players[packet->attackid].sEshot3.Z = packet->ez3;
		PlayerInfo.players[packet->attackid].sEshot4.X = packet->ex4;
		PlayerInfo.players[packet->attackid].sEshot4.Y = packet->ey4;
		PlayerInfo.players[packet->attackid].sEshot4.Z = packet->ez4;
		PlayerInfo.players[packet->attackid].sEshot5.X = packet->ex5;
		PlayerInfo.players[packet->attackid].sEshot5.Y = packet->ey5;
		PlayerInfo.players[packet->attackid].sEshot5.Z = packet->ez5;
		PlayerInfo.players[packet->attackid].sEshot6.X = packet->ex6;
		PlayerInfo.players[packet->attackid].sEshot6.Y = packet->ey6;
		PlayerInfo.players[packet->attackid].sEshot6.Z = packet->ez6;
		PlayerInfo.players[packet->attackid].sEshot7.X = packet->ex7;
		PlayerInfo.players[packet->attackid].sEshot7.Y = packet->ey7;
		PlayerInfo.players[packet->attackid].sEshot7.Z = packet->ez7;
		PlayerInfo.players[packet->attackid].sEshot8.X = packet->ex8;
		PlayerInfo.players[packet->attackid].sEshot8.Y = packet->ey8;
		PlayerInfo.players[packet->attackid].sEshot8.Z = packet->ez8;
		PlayerInfo.players[packet->attackid].sfired = true;
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
		PlayerInfo.players[packet->attack_id].weptype = packet->wep_type;

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
void ClientSocket::Send_Fire_Effect(int attack_id, FVector ImLoc, FRotator ImRot, int wtype)
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
	packet.wep_type = wtype;
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
void ClientSocket::Send_ShotGun_packet(int attack_id, TArray<FVector> ServerBeamStart, TArray<FVector> ServerBeamEnd, int size)
{
	CS_SHOTGUN_BEAM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_SHOTGUN_BEAM;
	packet.attackid = attack_id;
	//ServerBeamStart.SetNum(size);
	//ServerBeamEnd.SetNum(size);
	packet.sx = ServerBeamStart[0].X;
	packet.sy = ServerBeamStart[0].Y;
	packet.sz = ServerBeamStart[0].Z;

	packet.ex0 = ServerBeamEnd[0].X;
	packet.ey0 = ServerBeamEnd[0].Y;
	packet.ez0 = ServerBeamEnd[0].Z;
	packet.ex1 = ServerBeamEnd[1].X;
	packet.ey1 = ServerBeamEnd[1].Y;
	packet.ez1 = ServerBeamEnd[1].Z;
	packet.ex2 = ServerBeamEnd[2].X;
	packet.ey2 = ServerBeamEnd[2].Y;
	packet.ez2 = ServerBeamEnd[2].Z;
	packet.ex3 = ServerBeamEnd[3].X;
	packet.ey3 = ServerBeamEnd[3].Y;
	packet.ez3 = ServerBeamEnd[3].Z;
	packet.ex4 = ServerBeamEnd[4].X;
	packet.ey4 = ServerBeamEnd[4].Y;
	packet.ez4 = ServerBeamEnd[4].Z;
	packet.ex5 = ServerBeamEnd[5].X;
	packet.ey5 = ServerBeamEnd[5].Y;
	packet.ez5 = ServerBeamEnd[5].Z;
	packet.ex6 = ServerBeamEnd[6].X;
	packet.ey6 = ServerBeamEnd[6].Y;
	packet.ez6 = ServerBeamEnd[6].Z;
	packet.ex7 = ServerBeamEnd[7].X;
	packet.ey7 = ServerBeamEnd[7].Y;
	packet.ez7 = ServerBeamEnd[7].Z;

	packet.ex8 = ServerBeamEnd[8].X;
	packet.ey8 = ServerBeamEnd[8].Y;
	packet.ez8 = ServerBeamEnd[8].Z;
	/*packet.ex9 = ServerBeamEnd[9].X;
	packet.ey9 = ServerBeamEnd[9].Y;
	packet.ez9 = ServerBeamEnd[9].Z;*/
	SendPacket(&packet);
}
void ClientSocket::Send_ShotGun_damaged_packet(int damaged_id1, int damaged_id2, int damaged_id3, float damaged1, float damaged2, float damaged3)
{
	CS_SHOTGUN_DAMAGED_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_SHOTGUN_DAMAGED;
	packet.damaged_id = damaged_id1;
	packet.damaged_id1 = damaged_id2;
	packet.damaged_id2 = damaged_id3;
	packet.damage = damaged1;
	packet.damage1 = damaged2;
	packet.damage2 = damaged3;

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