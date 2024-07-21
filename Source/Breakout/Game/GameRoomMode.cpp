// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameRoomMode.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
AGameRoomMode::AGameRoomMode()
{
	bUseSeamlessTravel = true;
}

void AGameRoomMode::Tick(float Delta)
{
	Super::Tick(Delta);

	if (true == Cast<UBOGameInstance>(GetGameInstance())->m_Socket->bTravel) {
		GetWorld()->ServerTravel(FString("/Game/Maps/MainMap"), true);
		//SleepEx(0.5, true);
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->bTravel = false;
	}
}
