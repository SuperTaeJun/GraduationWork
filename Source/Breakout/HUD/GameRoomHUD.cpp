// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/GameRoomHUD.h"
#include "HUD/SelectCharacterUi.h"

void AGameRoomHUD::BeginPlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	SelectCharacter = CreateWidget<USelectCharacterUi>(PlayerController, SelectCharacterClass); 

	FInputModeUIOnly UiGameInput;
	PlayerController->SetInputMode(UiGameInput);

	AddSelectCharacter();
}

void AGameRoomHUD::AddSelectCharacter()
{
	SelectCharacter->AddToViewport();
}
