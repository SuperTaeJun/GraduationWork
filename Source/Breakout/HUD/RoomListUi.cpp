// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/RoomListUi.h"
#include "Components/Button.h"
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
	RemoveFromParent();
}

void URoomListUi::SlotTwoPress()
{
	RemoveFromParent();
}

void URoomListUi::SlotThreePress()
{
	RemoveFromParent();
}

void URoomListUi::SlotFourPress()
{
	RemoveFromParent();
}
