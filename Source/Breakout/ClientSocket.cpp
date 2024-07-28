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
		gameinst->SetPlayerID(packet->id);
		bLoginConnect = packet->bLogin;
		break;
	}
	case SC_LOGIN_FAIL: {
		SC_LOGIN_FAIL_PACKET* packet = reinterpret_cast<SC_LOGIN_FAIL_PACKET*>(ptr);
		switch (packet->failType)
		{
		case OVERLAP_ID:
		{
			UE_LOG(LogTemp, Warning, TEXT("OVERLAP_ID"));
			failType = 0;
			break;
		}
		case WRONG_ID:
		{
			UE_LOG(LogTemp, Warning, TEXT("WRONG ID"));
			failType = 1;
			break;
		}
		case WRONG_PW:
		{
			failType = 2;
			UE_LOG(LogTemp, Warning, TEXT("WRONG PW"));
			break;
		}
		default:
			break;
		}
		break;
	}
	case SC_LOBBY_ROOM:
	{
		SC_LOBBY_PACKET* packet = reinterpret_cast<SC_LOBBY_PACKET*>(ptr);
		binLobby = packet->bLobby;
		break;
	}
	case SC_ITEM: {
		SC_ITEM_PACKET* packet = reinterpret_cast<SC_ITEM_PACKET*>(ptr);
		auto info = make_shared<CItem>();
		info->Id = packet->id;
		info->X = packet->x;
		info->Y = packet->y;
		info->Z = packet->z;
		ItemQueue.push(info);
		break;
	}
	case SC_OTHER_PLAYER:
	{
		SC_PLAYER_SYNC* packet = reinterpret_cast<SC_PLAYER_SYNC*>(ptr);
		auto info = make_shared<CPlayer>();
		info->Id = packet->id;
		info->X = packet->x;
		info->Y = packet->y;
		info->Z = packet->z;
		info->Yaw = packet->yaw;
		info->p_type = packet->p_type;
		info->userId =  packet->name;
		Tempname.push(packet->name);
		UE_LOG(LogTemp, Warning, TEXT("other name : %s"), packet->name);
		MyCharacterController->SetNewCharacterInfo(info);
		
		break;
	}
	case SC_MOVE_PLAYER:
	{
		CS_MOVE_PACKET* packet = reinterpret_cast<CS_MOVE_PACKET*>(ptr);
		PlayerInfo.players[packet->id].X = packet->x;
		PlayerInfo.players[packet->id].Y = packet->y;
		PlayerInfo.players[packet->id].Z = packet->z;
		PlayerInfo.players[packet->id].Yaw = packet->yaw;
		PlayerInfo.players[packet->id].VeloX = packet->vx;
		PlayerInfo.players[packet->id].VeloY = packet->vy;
		PlayerInfo.players[packet->id].VeloZ = packet->vz;
		PlayerInfo.players[packet->id].Max_Speed = packet->Max_speed;
		PlayerInfo.players[packet->id].AO_PITCH = packet->AO_pitch;
		PlayerInfo.players[packet->id].AO_YAW = packet->AO_yaw;
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
		MyCharacterController->SetPlayerInfo(&PlayerInfo);
		MyCharacterController->SetInitPlayerInfo(player);
		break;
	}
	case SC_OTHER_WEAPO: {
		SC_SYNC_WEAPO* packet = reinterpret_cast<SC_SYNC_WEAPO*>(ptr);
		PlayerInfo.players[packet->id].w_type = packet->weapon_type;

		PlayerInfo.players[packet->id].bselectweapon = true;
		break;
	}
	case SC_ALL_READY: {
		SC_ACCEPT_READY* packet = reinterpret_cast<SC_ACCEPT_READY*>(ptr);
		bAllReady = packet->ingame;
		break;
	}
	case SC_TRAVLE: {
		SC_TRAVEL_PACKET* packet = reinterpret_cast<SC_TRAVEL_PACKET*>(ptr);
		bTravel = packet->ingame;
		break;
	}
  // 공격 나이아가라 이팩트 효과
	case SC_ATTACK: {
		SC_ATTACK_PLAYER* packet = reinterpret_cast<SC_ATTACK_PLAYER*>(ptr);
		PlayerInfo.players[packet->clientid].Sshot.X = packet->sx;
		PlayerInfo.players[packet->clientid].Sshot.Y = packet->sy;
		PlayerInfo.players[packet->clientid].Sshot.Z = packet->sz;
		PlayerInfo.players[packet->clientid].Eshot.X = packet->ex;
		PlayerInfo.players[packet->clientid].Eshot.Y = packet->ey;
		PlayerInfo.players[packet->clientid].Eshot.Z = packet->ez;
		MyCharacterController->SetAttack(packet->clientid);
		break;
	}
	case SC_SHOTGUN_BEAM: {
		CS_SHOTGUN_BEAM_PACKET* packet = reinterpret_cast<CS_SHOTGUN_BEAM_PACKET*>(ptr);
		PlayerInfo.players[packet->attackid].sSshot.X = packet->sx;
		PlayerInfo.players[packet->attackid].sSshot.Y = packet->sy;
		PlayerInfo.players[packet->attackid].sSshot.Z = packet->sz;
		PlayerInfo.players[packet->attackid].sEshot.Pitch = packet->pitch0;
		PlayerInfo.players[packet->attackid].sEshot.Yaw = packet->yaw0;
		PlayerInfo.players[packet->attackid].sEshot.Roll= packet->roll0;
		PlayerInfo.players[packet->attackid].sEshot1.Pitch = packet->pitch1;
		PlayerInfo.players[packet->attackid].sEshot1.Yaw = packet->yaw1;
		PlayerInfo.players[packet->attackid].sEshot1.Roll = packet->roll1;
		PlayerInfo.players[packet->attackid].sEshot2.Pitch = packet->pitch2;
		PlayerInfo.players[packet->attackid].sEshot2.Yaw = packet->yaw2;
		PlayerInfo.players[packet->attackid].sEshot2.Roll = packet->roll2;
		PlayerInfo.players[packet->attackid].sEshot3.Pitch = packet->pitch3;
		PlayerInfo.players[packet->attackid].sEshot3.Yaw = packet->yaw3;
		PlayerInfo.players[packet->attackid].sEshot3.Roll = packet->roll3;
		PlayerInfo.players[packet->attackid].sEshot4.Pitch = packet->pitch4;
		PlayerInfo.players[packet->attackid].sEshot4.Yaw = packet->yaw4;
		PlayerInfo.players[packet->attackid].sEshot4.Roll = packet->roll4;
		PlayerInfo.players[packet->attackid].sfired = true;
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
		PlayerInfo.players[packet->attack_id].weptype = packet->wep_type;
		MyCharacterController->SetHitEffect(packet->attack_id);
	
		break;
	}
	case SC_BOJOWEAPON: {
		CS_BOJOWEAPON_PACKET* packet = reinterpret_cast<CS_BOJOWEAPON_PACKET*>(ptr);
		PlayerInfo.players[packet->attack_id].Hshot.X = packet->lx;
		PlayerInfo.players[packet->attack_id].Hshot.Y = packet->ly;
		PlayerInfo.players[packet->attack_id].Hshot.Z = packet->lz;
		PlayerInfo.players[packet->attack_id].FEffect.Pitch = packet->r_pitch;
		PlayerInfo.players[packet->attack_id].FEffect.Yaw = packet->r_yaw;
		PlayerInfo.players[packet->attack_id].FEffect.Roll = packet->r_roll;
		PlayerInfo.players[packet->attack_id].bojotype = packet->wep_type;
		break;
	}
	case SC_BOJO_ANIM: {
		CS_BOJO_ANIM_PACKET* packet = reinterpret_cast<CS_BOJO_ANIM_PACKET*>(ptr);
		PlayerInfo.players[packet->id].bojoanimtype = packet->bojoanimtype;
		break;
	}
	//HP동기화 처리
	case SC_HP_CHANGE: {
		SC_HP_CHANGE_PACKET* packet = reinterpret_cast<SC_HP_CHANGE_PACKET*>(ptr);
		PlayerInfo.players[packet->id].hp = packet->HP;
		PlayerInfo.players[packet->id].bAlive = packet->bAlive;
		//hp = packet->HP;
		break;
	}
	case SC_NiAGARA: {
	
		CS_NIAGARA_SYNC_PACKET* packet = reinterpret_cast<CS_NIAGARA_SYNC_PACKET*>(ptr);
		PlayerInfo.players[packet->id].p_type = packet->playertype;
		PlayerInfo.players[packet->id].bniagara = true;
		PlayerInfo.players[packet->id].skilltype = packet->num;
		break;
	}
	case SC_NiAGARA_CANCEL: {
		CS_NIAGARA_CANCEL_PACKET* packet = reinterpret_cast<CS_NIAGARA_CANCEL_PACKET*>(ptr);
		PlayerInfo.players[packet->id].bniagara = false;
		PlayerInfo.players[packet->id].skilltype = packet->num;
		break;
	}
	case SC_NiAGARA_CH1: {
		CS_NIAGARA_PACKETCH1* packet = reinterpret_cast<CS_NIAGARA_PACKETCH1*>(ptr);
		PlayerInfo.players[packet->id].p_type = packet->playertype;
		PlayerInfo.players[packet->id].CH1NiaLoc.X = packet->x;
		PlayerInfo.players[packet->id].CH1NiaLoc.Y = packet->y;
		PlayerInfo.players[packet->id].CH1NiaLoc.Z = packet->z;
		PlayerInfo.players[packet->id].skilltype = packet->num;
		break;
	}
	case SC_SIGNAL: {
		CS_SIGNAL_PACKET* packet = reinterpret_cast<CS_SIGNAL_PACKET*>(ptr);
		PlayerInfo.players[packet->id].skilltype = packet->num;
		break;
	}
	case SC_END_GAME: {
		CS_END_GAME_PACKET* packet = reinterpret_cast<CS_END_GAME_PACKET*>(ptr);
		PlayerInfo.players[packet->id].bEndGame = true;
		PlayerInfo.players[packet->id].WinnerID = packet->winnerid;
		bEndGame = packet->bEND;
		bName = false;
		break;
	}
	case SC_MYITEM_COUNT:{
		//필요 없는 패킷
		SC_MY_ITEM_COUNT* packet = reinterpret_cast<SC_MY_ITEM_COUNT*>(ptr);
		//Tempcnt = packet->MyITEMCount;
		bAcquire = true;
		break;
	}
	case SC_ITEM_ACQUIRE: {
		SC_ITEM_ACQUIRE_PACKET* packet = reinterpret_cast<SC_ITEM_ACQUIRE_PACKET*>(ptr);
		TempPlayerName = packet->cid;
		Tempcnt2 = packet->itemCount;
		bAcquire = true;
		PlayerInfo.players[packet->id].itemCount = packet->itemCount;
		bitemcount = true;
		break;
	}
	case SC_ALIVE: {
		CS_ALIVE_PACKET* packet = reinterpret_cast<CS_ALIVE_PACKET*>(ptr);
		PlayerInfo.players[packet->id].deadtype = packet->deadtype;
		break;
	}
	case SC_DISSOLVE: {
		CS_DISSOLVE_PACKET* packet = reinterpret_cast<CS_DISSOLVE_PACKET*>(ptr);
		PlayerInfo.players[packet->id].dissolve = packet->dissolve;
		break;
	}
	case SC_REMOVE_ITEM: {
		CS_REMOVE_ITEM_PACKET* packet = reinterpret_cast<CS_REMOVE_ITEM_PACKET*>(ptr);
		MyCharacterController->SetDestroyItemid(packet->itemid);
		PlayerInfo.players[packet->id].bDestroyItem = true;
		break;
	}
	case SC_INCREASE_COUNT: {
		CS_INCREASE_ITEM_PACKET* packet = reinterpret_cast<CS_INCREASE_ITEM_PACKET*>(ptr);
		// cid 굳이 보낼 필요 없을 듯 그냥 players[packet-cid].name 으로 하면 될듯
		TempPlayerName = packet->cid;
		Tempcnt2 = packet->itemCount;
		bAcquire = true;
		PlayerInfo.players[packet->Increaseid].itemCount = packet->itemCount;
		bitemcount = true;
		break;
	}
	case SC_DECREASE: {
		CS_DECREASE_ITEM_PACKET* packet = reinterpret_cast<CS_DECREASE_ITEM_PACKET*>(ptr);
		TempPlayerName = packet->cid;
		Tempcnt2 = packet->itemCount;
		bAcquire = true;
		PlayerInfo.players[packet->Increaseid].itemCount = packet->itemCount;
		bitemcount = true;
		break;
	}
	case SC_WALL: {
		CS_WALL_PACKET* packet = reinterpret_cast<CS_WALL_PACKET*>(ptr);
		PlayerInfo.players[packet->id].bulletWallid = packet->wall_id;
		PlayerInfo.players[packet->id].BulletLoc.X = packet->lx;
		PlayerInfo.players[packet->id].BulletLoc.Y = packet->ly;
		PlayerInfo.players[packet->id].BulletLoc.Z = packet->lz;
		PlayerInfo.players[packet->id].BulletRot.Roll = packet->r_roll;
		PlayerInfo.players[packet->id].BulletRot.Pitch = packet->r_pitch;
		PlayerInfo.players[packet->id].BulletRot.Yaw = packet->r_yaw;
		PlayerInfo.players[packet->id].bBulletWall = true;
		break;
	}
	case SC_RELOAD: {
		CS_RELOAD_PACKET* packet = reinterpret_cast<CS_RELOAD_PACKET*>(ptr);
		PlayerInfo.players[packet->id].bServerReload = packet->bReload;
		break;
	}
	case SC_ITEM_ANIM: {
		CS_ITEM_ANIM_PACKET* packet = reinterpret_cast<CS_ITEM_ANIM_PACKET*>(ptr);
		PlayerInfo.players[packet->id].itemAnimtype = packet->num;
		break;
	}
	case SC_REMOVE_WEAPON: {
		CS_REMOVE_WEAPON_PACKET* packet = reinterpret_cast<CS_REMOVE_WEAPON_PACKET*>(ptr);
		PlayerInfo.players[packet->id].bGetWeapon = packet->bWeapon;
		break;
	}
	case SC_MYNEW_COUNT: {
		SC_MYNEW_ITEM_COUNT* packet = reinterpret_cast<SC_MYNEW_ITEM_COUNT*>(ptr);
		//UE_LOG(LogTemp, Warning, TEXT("packet->mynewcount  : %d"), packet->MyITEMCount);

		// 화면 하단 UI
		MyItemCount = packet->MyITEMCount;
		itemflag = true;
		break;
	}
	case SC_CH2_SKILL: {
		SC_CH2_SKILL_PACKET* packet = reinterpret_cast<SC_CH2_SKILL_PACKET*>(ptr);
		PlayerInfo.players[packet->id].p_type = packet->p_type;
		PlayerInfo.players[packet->id].bFinishSkill = packet->bfinish;
		break;
	}
	case SC_MOPP:
	{
		CS_MOPP_PACKET* packet = reinterpret_cast<CS_MOPP_PACKET*>(ptr);
		MyCharacterController->SetMoppItemID(packet->itemid);
		MoppType = packet->mopptype;
		break;
	}
	case SC_LIGHT: {
		CS_LIGHT_ON_PACKET* packet = reinterpret_cast<CS_LIGHT_ON_PACKET*>(ptr);
		PlayerInfo.players[packet->id].bLightOn = packet->bLight;
		break;
	}
	case SC_RECHARGE: {
		CS_RECHARGE_PACKET* packet = reinterpret_cast<CS_RECHARGE_PACKET*>(ptr);
		PlayerInfo.players[packet->id].bRecharge = packet->bRecharge;
		break;
	}
	case SC_HIT_ANIM: {
		CS_HIT_ANIM_PACKET* packet = reinterpret_cast<CS_HIT_ANIM_PACKET*>(ptr);
		PlayerInfo.players[packet->id].bHitAnim = packet->bHitAnim;
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

void ClientSocket::Send_Account_PACKET(char* id, char* pw)
{
	CS_ACCOUNT_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_ACCOUNT;
	strcpy_s(packet.id, id);
	strcpy_s(packet.pw, pw);

	SendPacket(&packet);
}

void ClientSocket::Send_Move_Packet(int sessionID, FVector Location, FRotator Rotation, FVector Velocity, float Max_speed, float AO_Yaw, float AO_Pitch)
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
	packet.AO_yaw = AO_Yaw;
	packet.AO_pitch = AO_Pitch;

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
	packet.bselectwep = true;
	SendPacket(&packet);
}
void ClientSocket::Send_Ready_Packet(bool ready, int id)
{
	CS_READY_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_READY;
	packet.id = id;
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
void ClientSocket::Send_BojoWeapon_packet(int id, FVector ImLoc, FRotator ImRot, int wtype)
{
	CS_BOJOWEAPON_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_BOJOWEAPON;
	packet.attack_id = id;
	packet.lx = ImLoc.X;
	packet.ly = ImLoc.Y;
	packet.lz = ImLoc.Z;
	packet.r_pitch = ImRot.Pitch;
	packet.r_yaw = ImRot.Yaw;
	packet.r_roll = ImRot.Roll;
	packet.wep_type = wtype;
	SendPacket(&packet);
}



void ClientSocket::Send_ShotGun_packet(int attack_id, FVector ServerBeamStart, TArray<FRotator> ServerBeamEnd, int size)
{
	CS_SHOTGUN_BEAM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_SHOTGUN_BEAM;
	packet.attackid = attack_id;
	ServerBeamEnd.SetNum(size);
	packet.sx = ServerBeamStart.X;
	packet.sy = ServerBeamStart.Y;
	packet.sz = ServerBeamStart.Z;

	packet.pitch0 = ServerBeamEnd[0].Pitch;
	packet.yaw0 = ServerBeamEnd[0].Yaw;
	packet.roll0 = ServerBeamEnd[0].Roll;
	packet.pitch1 = ServerBeamEnd[1].Pitch;
	packet.yaw1 = ServerBeamEnd[1].Yaw;
	packet.roll1 = ServerBeamEnd[1].Roll;
	packet.pitch2 = ServerBeamEnd[2].Pitch;
	packet.yaw2 = ServerBeamEnd[2].Yaw;
	packet.roll2 = ServerBeamEnd[2].Roll;
	packet.pitch3 = ServerBeamEnd[3].Pitch;
	packet.yaw3 = ServerBeamEnd[3].Yaw;
	packet.roll3 = ServerBeamEnd[3].Roll;
	packet.pitch4 = ServerBeamEnd[4].Pitch;
	packet.yaw4 = ServerBeamEnd[4].Yaw;
	packet.roll4 = ServerBeamEnd[4].Roll;
	SendPacket(&packet);
}
void ClientSocket::Send_Niagara_packet(int clientid, PlayerType type, int num)
{
	UE_LOG(LogClass, Warning, TEXT("BNIAGAR "));
	CS_NIAGARA_SYNC_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_NiAGARA;
	packet.id = clientid;
	packet.playertype = type;
	packet.num = num;
	SendPacket(&packet);
}
void ClientSocket::Send_Niagara_cancel(bool bcancel, int id,int num)
{
	CS_NIAGARA_CANCEL_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_NiAGARA_CANCEL;
	packet.cancel = bcancel;
	packet.id = id;
	packet.num = num;
	SendPacket(&packet);

}
void ClientSocket::Send_Niagara_packetch1(int clinetid, PlayerType type, FVector loc, int num)
{
	CS_NIAGARA_PACKETCH1 packet;
	packet.size = sizeof(packet);
	packet.type = CS_NiAGARA_CH1;
	packet.id = clinetid;
	packet.playertype = type;
	packet.x = loc.X;
	packet.y = loc.Y;
	packet.z = loc.Z;
	packet.num = num;
	SendPacket(&packet);
}
void ClientSocket::Send_Start_game_packet(int id)
{
	CS_START_GAME_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_START_GAME;
	packet.id = id;
	SendPacket(&packet);

}
void ClientSocket::Send_item_info_packet(int objid)
{
	UE_LOG(LogTemp, Warning, TEXT("count"));
	CS_ITEM_INFO_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_ITEM_INFO;
	packet.objid = objid;
	SendPacket(&packet);
}
void ClientSocket::Send_End_Game_packet(int id, bool bPlay)
{
	CS_END_GAME_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_END_GAME;
	packet.id = id;
	packet.bPlay = bPlay;
	SendPacket(&packet);
}
void ClientSocket::Send_Signal_packet(int id, int num)
{
	CS_SIGNAL_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_SIGNAl;
	packet.id = id;
	packet.num = num;
	SendPacket(&packet);
}
void ClientSocket::Send_Item_packet(int id, int itemCount)
{
	//내가 아이템 먹었을 때
	CS_ITEM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_GETITEM;
	packet.id = id;
	packet.itemCount = itemCount;
	SendPacket(&packet);
}
void ClientSocket::Send_Alive_packet(int id, int deadtype)
{
	CS_ALIVE_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_ALIVE;
	packet.id = id;
	packet.deadtype = deadtype;
	SendPacket(&packet);
}
void ClientSocket::Send_Destroyed_item_packet(int id, int playerid)
{
	CS_REMOVE_ITEM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_REMOVE_ITEM;
	packet.itemid = id;
	packet.id = playerid;
	SendPacket(&packet);
}
void ClientSocket::Send_Increase_item_count_packet(int id, int itemcount)
{
	CS_INCREASE_ITEM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_INCREASE_COUNT;
	packet.Increaseid = id;
	packet.itemCount = itemcount;
	SendPacket(&packet);
}
void ClientSocket::Send_Decrease_item_count_packet(int id, int itemcount)
{
	CS_DECREASE_ITEM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_DECREASE_COUNT;
	packet.Increaseid = id;
	packet.itemCount = itemcount;
	SendPacket(&packet);
}
void ClientSocket::Send_Reload_packet(int id, bool bReload)
{
	CS_RELOAD_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_RELOAD;
	packet.id = id;
	packet.bReload = bReload;
	SendPacket(&packet);
}
void ClientSocket::Send_item_Anim_packet(int id, int num)
{
	CS_ITEM_ANIM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_ITEM_ANIM;
	packet.id = id;
	packet.num = num;
	SendPacket(&packet);

}
void ClientSocket::Send_Remove_Weapon(int id, bool bWeapon)
{
	CS_REMOVE_WEAPON_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_REMOVE_WEAPON;
	packet.id = id;
	packet.bWeapon = bWeapon;
	SendPacket(&packet);
}
void ClientSocket::Send_CH2_SKILL_PACKET(int id, PlayerType type, bool bSkill)
{
	SC_CH2_SKILL_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_CH2_SKILL;
	packet.id = id;
	packet.p_type = type;
	packet.bfinish = bSkill;
	SendPacket(&packet);
}
void ClientSocket::Send_My_HP_PACKET(int id, float damaage, bool bAlive)
{
	CS_DAMAGE_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_DAMAGE;
	packet.id = id;
	packet.hp = damaage;
	packet.bAlive = bAlive;
	SendPacket(&packet);
}
void ClientSocket::Send_Dissolve_packet(int id, int dissolve)
{
	CS_DISSOLVE_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_DISSOLVE;
	packet.id = id;
	packet.dissolve = dissolve;
	SendPacket(&packet);
}
void ClientSocket::Send_BojoAnim_packet(int id, int bojo)
{
	CS_BOJO_ANIM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_BOJO_ANIM;
	packet.id = id;
	packet.bojoanimtype = bojo;
	SendPacket(&packet);
}

void ClientSocket::Send_Mopp_Sync_packet(int itemid, int mopptype, int id)
{
	CS_MOPP_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_MOPP;
	packet.itemid = itemid;
	packet.id = id;
	packet.mopptype = mopptype;
	SendPacket(&packet);
}

void ClientSocket::Send_BulletWall_packet(int id, int Wallid, FVector WLoc, FRotator WRot)
{
	CS_WALL_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_BULLET_WALL;
	packet.wall_id = Wallid;
	packet.lx = WLoc.X;
	packet.ly = WLoc.Y;
	packet.lz = WLoc.Z;
	packet.r_pitch = WRot.Pitch;
	packet.r_yaw = WRot.Yaw;
	packet.r_roll = WRot.Roll;
	packet.id = id;
	SendPacket(&packet);
}

void ClientSocket::Send_Light_On_packet(int id, bool bLight)
{
	CS_LIGHT_ON_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_LIGHT;
	packet.id = id;
	packet.bLight = bLight;
	SendPacket(&packet);
}

void ClientSocket::Send_Lobby_Room_pakcet(int id, int RoomNum)
{
	
	CS_LOBBY_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_ROOM;
	packet.id = id;
	packet.RoomNum = RoomNum;
	SendPacket(&packet);
}

void ClientSocket::Send_Hovered_packet(int id, int RoomNum)
{
	CS_LOBBY_HOVER packet;
	packet.size = sizeof(packet);
	packet.type = CS_HOVER;
	packet.id = id;
	packet.RoomNum = RoomNum;
	SendPacket(&packet);
}

void ClientSocket::Send_Recharge_packet(int id, bool bRecharge)
{
	CS_RECHARGE_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_RECHARGE;
	packet.id = id;
	packet.bRecharge = bRecharge;
	SendPacket(&packet);
}

void ClientSocket::Send_Hit_Anim_packet(int id, bool bHitAnim)
{
	CS_HIT_ANIM_PACKET packet;
	packet.size = sizeof(packet);
	packet.type = CS_HIT_ANIM;
	packet.id = id;
	packet.bHitAnim = bHitAnim;
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

	Iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(ServerSocket), Iocp, 0, 0);

	RecvPacket();

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