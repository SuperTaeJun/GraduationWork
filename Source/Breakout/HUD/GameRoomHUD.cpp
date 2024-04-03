// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/GameRoomHUD.h"
#include "HUD/SelectCharacterUi.h"
#include "HUD/Login.h"

void AGameRoomHUD::BeginPlay()
{
	AddSelectCharacter();
	AddLoginUi();

}

void AGameRoomHUD::AddLoginUi()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		LoginUi = CreateWidget<ULogin>(PlayerController, LoginUiClass);

		FInputModeUIOnly UiGameInput;
		PlayerController->SetInputMode(UiGameInput);
		if (LoginUi)
			LoginUi->AddToViewport();
	}
}

void AGameRoomHUD::AddSelectCharacter()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		SelectCharacter = CreateWidget<USelectCharacterUi>(PlayerController, SelectCharacterClass);

		FInputModeUIOnly UiGameInput;
		PlayerController->SetInputMode(UiGameInput);
		if(SelectCharacter)
			SelectCharacter->AddToViewport();
	}
}
