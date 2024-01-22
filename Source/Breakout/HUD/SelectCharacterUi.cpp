// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SelectCharacterUi.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Game/BOGameInstance.h"
void USelectCharacterUi::NativeConstruct()
{
	Character1Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character1ButtonPressed);
	Character2Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character2ButtonPressed);
	Character3Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character3ButtonPressed);
	Character4Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character4ButtonPressed);
}

void USelectCharacterUi::Character1ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter1);

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Testmap"));
}

void USelectCharacterUi::Character2ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter2);

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Testmap"));
}

void USelectCharacterUi::Character3ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter3);

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Testmap"));
}

void USelectCharacterUi::Character4ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter4);

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Testmap"));
}
