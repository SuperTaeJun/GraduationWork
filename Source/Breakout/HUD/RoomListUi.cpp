// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/RoomListUi.h"
#include "Components/Button.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
#include "Sound/SoundCue.h"
void URoomListUi::NativeConstruct()
{
	if (SlotOne)
	{
		SlotOne->OnClicked.AddDynamic(this, &URoomListUi::SlotOnePress);
		SlotOne->OnHovered.AddDynamic(this, &URoomListUi::SlotOneHoverd);
	}
	if (SlotTwo)
	{
		SlotTwo->OnClicked.AddDynamic(this, &URoomListUi::SlotTwoPress);
		SlotTwo->OnHovered.AddDynamic(this, &URoomListUi::SlotTwoHoverd);
	}
	if (SlotThree)
	{
		SlotThree->OnClicked.AddDynamic(this, &URoomListUi::SlotThreePress);
		SlotThree->OnHovered.AddDynamic(this, &URoomListUi::SlotThreeHoverd);
	}
	if (SlotFour)
	{
		SlotFour->OnClicked.AddDynamic(this, &URoomListUi::SlotFourPress);
		SlotFour->OnHovered.AddDynamic(this, &URoomListUi::SlotFourHoverd);
	}

	Inst = Cast<UBOGameInstance>(GetGameInstance());
}

void URoomListUi::SlotOnePress()
{
	if (Inst && Inst->m_Socket->binLobby) {
		//패킷 보낼 곳(id, game방 번호)
		if (Inst->m_Socket)
			Inst->m_Socket->Send_Lobby_Room_pakcet(Inst->GetPlayerID(), 1);

		RemoveFromParent();
	}
	if (ClickSound)
	{
		PlaySound(ClickSound);
	}
}

void URoomListUi::SlotTwoPress()
{
	if (Inst && Inst->m_Socket->binLobby) {
		//패킷 보낼 곳(id, game방 번호)
		if (Inst->m_Socket)
			Inst->m_Socket->Send_Lobby_Room_pakcet(Inst->GetPlayerID(), 2);

		RemoveFromParent();
	}
	if (ClickSound)
	{
		PlaySound(ClickSound);
	}
}

void URoomListUi::SlotThreePress()
{
	if (Inst && Inst->m_Socket->binLobby) {
		//패킷 보낼 곳(id, game방 번호)
		if (Inst->m_Socket)
			Inst->m_Socket->Send_Lobby_Room_pakcet(Inst->GetPlayerID(), 3);

		RemoveFromParent();
	}
	if (ClickSound)
	{
		PlaySound(ClickSound);
	}
}

void URoomListUi::SlotFourPress()
{
	if (Inst && Inst->m_Socket->binLobby) {
		//패킷 보낼 곳(id, game방 번호)
		if (Inst->m_Socket)
			Inst->m_Socket->Send_Lobby_Room_pakcet(Inst->GetPlayerID(), 4);

		RemoveFromParent();
	}
	if (ClickSound)
	{
		PlaySound(ClickSound);
	}
}

void URoomListUi::SlotOneHoverd()
{	
	if(Inst)
		Inst->m_Socket->Send_Hovered_packet(Inst->GetPlayerID(), 1);
}

void URoomListUi::SlotTwoHoverd()
{
	if (Inst)
		Inst->m_Socket->Send_Hovered_packet(Inst->GetPlayerID(), 2);
}

void URoomListUi::SlotThreeHoverd()
{
	if (Inst)
		Inst->m_Socket->Send_Hovered_packet(Inst->GetPlayerID(), 3);
}

void URoomListUi::SlotFourHoverd()
{
	if (Inst)
		Inst->m_Socket->Send_Hovered_packet(Inst->GetPlayerID(), 4);
}
