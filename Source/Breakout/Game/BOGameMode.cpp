// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BOGameMode.h"
#include "Character/CharacterBase.h"
#include "Game/BOGameInstance.h"
#include "Character/CharacterBase.h"
#include "GameFramework/PlayerStart.h"

ABOGameMode::ABOGameMode()
{
	/*m_Socket = ClientSocket::GetSingleton();
	m_Socket->InitSocket();

	connect = m_Socket->Connect("127.0.0.1", 12345);
	if (connect)
	{
		m_Socket->StartListen();
		UE_LOG(LogClass, Log, TEXT("IOCP Server connect success!"));
	}
	else
	{
		UE_LOG(LogClass, Warning, TEXT("IOCP Server connect FAIL!"));
	}*/

	ConstructorHelpers::FClassFinder<ACharacterBase>Character1Ref(TEXT("/Game/BP/Character/BP_Character1.BP_Character1_C"));
	Character1 = Character1Ref.Class;
	ConstructorHelpers::FClassFinder<ACharacterBase>Character2Ref(TEXT("/Game/BP/Character/BP_Character2.BP_Character2_C"));
	Character2 = Character2Ref.Class;
	ConstructorHelpers::FClassFinder<ACharacterBase>Character3Ref(TEXT("/Game/BP/Character/BP_Character3.BP_Character3_C"));
	Character3 = Character3Ref.Class;
	ConstructorHelpers::FClassFinder<ACharacterBase>Character4Ref(TEXT("/Game/BP/Character/BP_Character4.BP_Character4_C"));
	Character4 = Character4Ref.Class;
}


void ABOGameMode::Respawn(ACharacter* RespawnedCh, AController* RespawnedController)
{
	if (RespawnedCh)
	{
		RespawnedCh->Reset();
		RespawnedCh->Destroy();
	}
	if (RespawnedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(RespawnedController, PlayerStarts[Selection]);
		//Cast<ACharacterBase>(RespawnedController->GetPawn())->SetWeaponUi(Cast<ACharacterController>(RespawnedController));
	}

}

UClass* ABOGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	Super::GetDefaultPawnClassForController_Implementation(InController);

	if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() == ECharacterType::ECharacter1)
	{
		return Character1;
	}
	else if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() == ECharacterType::ECharacter2)
	{
		return Character2;
	}
	else if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() == ECharacterType::ECharacter3)
	{
		return Character3;
	}
	else if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() == ECharacterType::ECharacter4)
	{
		return Character4;
	}
	else
		return nullptr;
}
