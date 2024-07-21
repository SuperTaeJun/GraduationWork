// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/RoomListUi.h"
#include "Components/Button.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
void URoomListUi::NativeConstruct()
{
	if (SlotOne)
		SlotOne->OnClicked.AddDynamic(this, &URoomListUi::SlotOnePress);
	if (SlotTwo)
		SlotTwo->OnClicked.AddDynamic(this, &URoomListUi::SlotTwoPress);
	if (SlotThree)
		SlotThree->OnClicked.AddDynamic(this, &URoomListUi::SlotThreePress);
	if (SlotFour)
		SlotFour->OnClicked.AddDynamic(this, &URoomListUi::SlotFourPress);

}

void URoomListUi::SlotOnePress()
{
	//패킷 보낼 곳(id, game방 번호)
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Lobby_Room_pakcet(Cast<UBOGameInstance>(GetGameInstance())->GetPlayerID(), 1);
	RemoveFromParent();
}

void URoomListUi::SlotTwoPress()
{
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Lobby_Room_pakcet(Cast<UBOGameInstance>(GetGameInstance())->GetPlayerID(), 2);
	RemoveFromParent();
}

void URoomListUi::SlotThreePress()
{
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Lobby_Room_pakcet(Cast<UBOGameInstance>(GetGameInstance())->GetPlayerID(), 3);
	RemoveFromParent();
}

void URoomListUi::SlotFourPress()
{
	if (Cast<UBOGameInstance>(GetGameInstance())->m_Socket)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Lobby_Room_pakcet(Cast<UBOGameInstance>(GetGameInstance())->GetPlayerID(), 4);
	RemoveFromParent();
}
