// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/GameRoomHUD.h"
#include "HUD/SelectCharacterUi.h"

void AGameRoomHUD::BeginPlay()
{
	AddSelectCharacter();
	

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
