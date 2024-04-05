// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameRoomMode.h"
#include "Game/BOGameInstance.h"
AGameRoomMode::AGameRoomMode()
{
	bUseSeamlessTravel = true;
}

void AGameRoomMode::Tick(float Delta)
{
	Super::Tick(Delta);

	//UE_LOG(LogTemp, Warning, TEXT("HAHAH"));
	//UE_LOG(LogTemp, Warning, TEXT("INST TICK"));
	if (true == Cast<UBOGameInstance>(GetGameInstance())->m_Socket->bAllReady)
		GetWorld()->ServerTravel(FString("/Game/Maps/MainMap"), true);
}
