// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SelectCharacterUi.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
#include "Network/PacketData.h"
void USelectCharacterUi::NativeConstruct()
{
	Character1Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character1ButtonPressed);
	Character2Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character2ButtonPressed);
	Character3Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character3ButtonPressed);
	Character4Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character4ButtonPressed);
}
// 캐릭터 선택 패킷 보내는 곳
void USelectCharacterUi::Character1ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter1);
	/*PlayerType type = Character1;
	c_socket->Send_Character_Type(type);*/
	//UGameplayStatics::OpenLevel(GetWorld(), TEXT("/Game/Maps/MainMap.MainMap"));
	//GetWorld()->ServerTravel(FString("/Game/Maps/Testmap"));
	//GetWorld()->SeamlessTravel(FString("/Game/Maps/Testmap?listen"));
	//UE_LOG(LogClass, Warning, TEXT("MY type : %d"), type);
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Testmap"));
}

void USelectCharacterUi::Character2ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter2);
	/*PlayerType type = Character2;
	c_socket->Send_Character_Type(type);*/
	//GetWorld()->ServerTravel(FString("/Game/Maps/Testmap"));
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Testmap"));
	//GetWorld()->SeamlessTravel(FString("/Game/Maps/Testmap?listen"));
	//UE_LOG(LogClass, Warning, TEXT("MY type : %d"), type);
}

void USelectCharacterUi::Character3ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter3);
	/*PlayerType type = Character3;
	c_socket->Send_Character_Type(type);*/
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Testmap"));
	//GetWorld()->ServerTravel(FString("/Game/Maps/Testmap"));
	//GetWorld()->SeamlessTravel(FString("/Game/Maps/Testmap?listen"));
	//UE_LOG(LogClass, Warning, TEXT("MY type : %d"), type);
}

void USelectCharacterUi::Character4ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter4);
	//PlayerType type = Character4;
	//c_socket->Send_Character_Type(type);
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Testmap"));
	//GetWorld()->ServerTravel(FString("/Game/Maps/Testmap"));
	//GetWorld()->SeamlessTravel(FString("/Game/Maps/Testmap?listen"));
	//UE_LOG(LogClass, Warning, TEXT("MY type : %d"), type);
}
