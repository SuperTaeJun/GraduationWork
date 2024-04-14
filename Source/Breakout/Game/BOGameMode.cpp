// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BOGameMode.h"
#include "Character/CharacterBase.h"
#include "Game/BOGameInstance.h"
#include "Character/CharacterBase.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CharacterController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
ABOGameMode::ABOGameMode()
{
	bUseSeamlessTravel = true;

	ConstructorHelpers::FClassFinder<ACharacterBase>Character1Ref(TEXT("/Game/BP/Character/BP_Character1.BP_Character1_C"));
	Character1 = Character1Ref.Class;
	ConstructorHelpers::FClassFinder<ACharacterBase>Character2Ref(TEXT("/Game/BP/Character/BP_Character2.BP_Character2_C"));
	Character2 = Character2Ref.Class;
	ConstructorHelpers::FClassFinder<ACharacterBase>Character3Ref(TEXT("/Game/BP/Character/BP_Character3.BP_Character3_C"));
	Character3 = Character3Ref.Class;
	ConstructorHelpers::FClassFinder<ACharacterBase>Character4Ref(TEXT("/Game/BP/Character/BP_Character4.BP_Character4_C"));
	Character4 = Character4Ref.Class;
}

void ABOGameMode::BeginPlay()
{
	Super::BeginPlay();
	//DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	//GetWorldTimerManager().SetTimer(StartTimeHandle, this, &ABOGameMode::StartGame, 5.f);
}

void ABOGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ABOGameMode::Respawn(ACharacter* RespawnedCh, AController* RespawnedController, FName TagName)
{
	if (RespawnedCh)
	{

		RespawnedCh->Reset();
		RespawnedCh->Destroy();
	}
	if (RespawnedController)
	{
		FName Tagname = TagName;
		AActor* PlayerStarts;
		//AActor* PlayerStarts;
		//UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
		//UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

		PlayerStarts=FindPlayerStart(RespawnedCh->GetController(), *Tagname.ToString());

		//int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(RespawnedController, PlayerStarts);
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

AActor* ABOGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	//Super::ChoosePlayerStart(Player);

	AActor* PlayerStarts1 = FindPlayerStart(Player, FString(TEXT("PlayerStart1")));
	AActor* PlayerStarts2 = FindPlayerStart(Player, FString(TEXT("PlayerStart2")));
	AActor* PlayerStarts3 = FindPlayerStart(Player, FString(TEXT("PlayerStart3")));
	AActor* PlayerStarts4 = FindPlayerStart(Player, FString(TEXT("PlayerStart4")));
	switch (Cast<UBOGameInstance>(GetGameInstance())->CharacterType)
	{
	case ECharacterType::ECharacter1:
		if(PlayerStarts1) return PlayerStarts1;
		break;
	case ECharacterType::ECharacter2:
		if (PlayerStarts2) return PlayerStarts2;
		break;
	case ECharacterType::ECharacter3:
		if (PlayerStarts3) return PlayerStarts3;
		break;
	case ECharacterType::ECharacter4:
		if (PlayerStarts4) return PlayerStarts4;
		break;
	default:
		if (PlayerStarts1) return PlayerStarts1;
		break;
	}
	return 	Super::ChoosePlayerStart(Player);
}

void ABOGameMode::StartGame()
{
	EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}
