// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/RoomListUi.h"
#include "Components/Button.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
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
	//패킷 보낼 곳(id, game방 번호)
	if (Inst->m_Socket)
		Inst->m_Socket->Send_Lobby_Room_pakcet(Cast<UBOGameInstance>(GetGameInstance())->GetPlayerID(), 1);
	if (Inst->m_Socket->binLobby)
	{
		RemoveFromParent();
		Inst->m_Socket->binLobby = false;
	}
}

void URoomListUi::SlotTwoPress()
{
	//if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
	//	Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Lobby_Room_pakcet(Cast<UBOGameInstance>(GetGameInstance())->GetPlayerID(), 2);
	//if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket->binLobby)
	//{
	//	RemoveFromParent();
	//	Cast<UBOGameInstance>(GetGameInstance())->m_Socket->binLobby = false;
	//}
	if(Inst)
		RemoveFromParent();
}

void URoomListUi::SlotThreePress()
{
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Lobby_Room_pakcet(Cast<UBOGameInstance>(GetGameInstance())->GetPlayerID(), 3);
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket->binLobby)
	{
		RemoveFromParent();
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->binLobby = false;
	}
}

void URoomListUi::SlotFourPress()
{
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Lobby_Room_pakcet(Cast<UBOGameInstance>(GetGameInstance())->GetPlayerID(), 4);
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket->binLobby)
	{
		RemoveFromParent();
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->binLobby = false;
	}
}

void URoomListUi::SlotOneHoverd()
{

}

void URoomListUi::SlotTwoHoverd()
{

}

void URoomListUi::SlotThreeHoverd()
{

}

void URoomListUi::SlotFourHoverd()
{

}
