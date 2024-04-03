// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/LoginHUD.h"
#include "HUD/Login.h"

void ALoginHUD::AddLoginUi()
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

void ALoginHUD::BeginPlay()
{
	Super::BeginPlay();

	AddLoginUi();

}
