// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Login.h"
#include "Components/Button.h"
#include "Components/EditableText.h"


void ULogin::NativeConstruct()
{
	Super::NativeConstruct();

	if (Login)
		Login->OnClicked.AddDynamic(this, &ULogin::PressLogin);

}

void ULogin::PressLogin()
{
	//UE_LOG(LogTemp, Warning, TEXT("ID : %s"), ID->GetText().ToString());
	//UE_LOG(LogTemp, Warning, TEXT("Password : %s"), Password->GetText().ToString());
	GetWorld()->ServerTravel(FString("/Game/Maps/GameRoom"), true);
}
