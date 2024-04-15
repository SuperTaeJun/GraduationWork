// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SelectCharacterUi.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/SkeletalMeshActor.h"

//#include "Network/PacketData.h"
void USelectCharacterUi::NativeConstruct()
{
	Character1Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character1ButtonPressed);
	Character2Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character2ButtonPressed);
	Character3Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character3ButtonPressed);
	Character4Button->OnClicked.AddDynamic(this, &USelectCharacterUi::Character4ButtonPressed);

	Character1Button->OnHovered.AddDynamic(this, &USelectCharacterUi::Button1Hovered);
	Character1Button->OnUnhovered.AddDynamic(this, &USelectCharacterUi::EndHovered);
	Character2Button->OnHovered.AddDynamic(this, &USelectCharacterUi::Button2Hovered);
	Character2Button->OnUnhovered.AddDynamic(this, &USelectCharacterUi::EndHovered);
	Character3Button->OnHovered.AddDynamic(this, &USelectCharacterUi::Button3Hovered);
	Character3Button->OnUnhovered.AddDynamic(this, &USelectCharacterUi::EndHovered);
	Character4Button->OnHovered.AddDynamic(this, &USelectCharacterUi::Button4Hovered);
	Character4Button->OnUnhovered.AddDynamic(this, &USelectCharacterUi::EndHovered);

	MatchingButton->OnClicked.AddDynamic(this, &USelectCharacterUi::Matching);

	//월드에있는 캐릭터메쉬 가져오기
	SetAllCharacterMeshWithTag();
	bClicked = false;
}

void USelectCharacterUi::SetAllCharacterMeshWithTag()
{
	FName Tag1 = FName(TEXT("Character1"));
	FName Tag2 = FName(TEXT("Character2"));
	FName Tag3 = FName(TEXT("Character3"));
	FName Tag4 = FName(TEXT("Character4"));
	TArray<AActor*> FindActor;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag1, FindActor);
	Character1Mesh = Cast<ASkeletalMeshActor>(FindActor[0]);

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag2, FindActor);
	Character2Mesh = Cast<ASkeletalMeshActor>(FindActor[0]);

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag3, FindActor);
	Character3Mesh = Cast<ASkeletalMeshActor>(FindActor[0]);

	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag4, FindActor);
	Character4Mesh = Cast<ASkeletalMeshActor>(FindActor[0]);
}

void USelectCharacterUi::Matching()
{
	if(bClicked){
		//Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Ready_Packet(bClicked);
		
		GetWorld()->ServerTravel(FString("/Game/Maps/MainMap"), true);
	}
		//GetWorld()->ServerTravel(FString("/Game/Maps/MainMap"), true);
}

// 캐릭터 선택 패킷 보내는 곳
void USelectCharacterUi::Character1ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter1);
	/*PlayerType type = Character1;
	c_socket->Send_Character_Type(type);*/
	Character1Mesh->SetActorHiddenInGame(false);
	Character2Mesh->SetActorHiddenInGame(true);
	Character3Mesh->SetActorHiddenInGame(true);
	Character4Mesh->SetActorHiddenInGame(true);

	bClicked = true;
}

void USelectCharacterUi::Character2ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter2);
	/*PlayerType type = Character2;
	/c_socket->Send_Character_Type(type);*/
	Character1Mesh->SetActorHiddenInGame(true);
	Character2Mesh->SetActorHiddenInGame(false);
	Character3Mesh->SetActorHiddenInGame(true);
	Character4Mesh->SetActorHiddenInGame(true);

	bClicked = true;
}

void USelectCharacterUi::Character3ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter3);
	/*PlayerType type = Character3;
	c_socket->Send_Character_Type(type);*/
	Character1Mesh->SetActorHiddenInGame(true);
	Character2Mesh->SetActorHiddenInGame(true);
	Character3Mesh->SetActorHiddenInGame(false);
	Character4Mesh->SetActorHiddenInGame(true);

	bClicked = true;
}

void USelectCharacterUi::Character4ButtonPressed()
{
	Cast<UBOGameInstance>(GetGameInstance())->SetCharacterType(ECharacterType::ECharacter4);
	//PlayerType type = Character4;
	//c_socket->Send_Character_Type(type);
	Character1Mesh->SetActorHiddenInGame(true);
	Character2Mesh->SetActorHiddenInGame(true);
	Character3Mesh->SetActorHiddenInGame(true);
	Character4Mesh->SetActorHiddenInGame(false);

	bClicked = true;
}

void USelectCharacterUi::EndHovered()
{
	if (Character1Mesh && Character2Mesh && Character3Mesh && Character4Mesh)
	{
		if (!bClicked)
		{
			Character1Mesh->SetActorHiddenInGame(true);
			Character2Mesh->SetActorHiddenInGame(true);
			Character3Mesh->SetActorHiddenInGame(true);
			Character4Mesh->SetActorHiddenInGame(true);
		}
	}
}

void USelectCharacterUi::Button1Hovered()
{
	if (!bClicked)
		Character1Mesh->SetActorHiddenInGame(false);
}

void USelectCharacterUi::Button2Hovered()
{
	if (!bClicked)
		Character2Mesh->SetActorHiddenInGame(false);
}

void USelectCharacterUi::Button3Hovered()
{
	if (!bClicked)
		Character3Mesh->SetActorHiddenInGame(false);
}

void USelectCharacterUi::Button4Hovered()
{
	if (!bClicked)
		Character4Mesh->SetActorHiddenInGame(false);
}
