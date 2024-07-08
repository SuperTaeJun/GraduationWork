// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BOGameMode.h"
#include "Character/CharacterBase.h"
#include "Game/BOGameInstance.h"
#include "Character/CharacterBase.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CharacterController.h"
#include "Kismet/GameplayStatics.h"
#include "ClientSocket.h"
#include "GameProp/EscapeTool.h"
#include "TimerManager.h"
#include "Player/CharacterController.h"

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

	bStarted = false;
}

void ABOGameMode::BeginPlay()
{
	Super::BeginPlay();
	inst = Cast<UBOGameInstance>(GetGameInstance());
	//DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	//GetWorldTimerManager().SetTimer(StartTimeHandle, this, &ABOGameMode::StartGame, 5.f);
	//TArray<AActor*> Actors;

	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEscapeTool::StaticClass(), Actors);
	//for (int i = 0; i < Actors.Num(); i++)
	//{
	//	if (Cast<AEscapeTool>(Actors[i]) && inst)
	//	{
	//		int objid = Cast<AEscapeTool>(Actors[i])->ItemID;
	//		inst->m_Socket->Send_item_info_packet(objid);
	//	
	//		EscapeTools.Add(Cast<AEscapeTool>(Actors[i]));
	//	}
	//}
	if (BackGroundSound)
	{
	}
}

void ABOGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//bool bAllConnect = false;
	//if (inst->m_Socket->bAllReady==true && !bStarted)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("ballready!!!!!!!!!!!!!!!!!"));
	//	DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	//	GetWorldTimerManager().SetTimer(StartTimeHandle, this, &ABOGameMode::StartGame, 5.f);
	//if (inst->m_Socket->ItemQueue.size())
	//{
	//	FVector itemLoc;
	//	itemLoc.X = inst->m_Socket->ItemQueue.front()->X;
	//	itemLoc.Y = inst->m_Socket->ItemQueue.front()->Y;
	//	itemLoc.Z = inst->m_Socket->ItemQueue.front()->Z;

	//	EscapeTools[inst->m_Socket->ItemQueue.front()->Id]->ItemID = inst->m_Socket->ItemQueue.front()->Id;
	//	EscapeTools[inst->m_Socket->ItemQueue.front()->Id]->SetActorLocation(itemLoc);
	//	inst->m_Socket->ItemQueue.front() = nullptr;
	//	inst->m_Socket->ItemQueue.pop();
	//}
	//}
}


void ABOGameMode::SetDamageInsigator(ACharacterBase* DamageInsigatorCh)
{
	DamageInsigator = DamageInsigatorCh;
	UE_LOG(LogTemp, Warning, TEXT("ID : %d"), DamageInsigatorCh->_SessionId);
}

void ABOGameMode::Respawn(ACharacter* RespawnedCh, AController* RespawnedController, FName TagName)
{
	ACharacterBase* MyCharacter=Cast<ACharacterBase>(RespawnedCh);
	if (DamageInsigator)
	{
		if (MyCharacter->GetEscapeToolNum() >= 10)
		{
			DamageInsigator->SetEscapeToolNum(DamageInsigator->GetEscapeToolNum() + 3);
			MyCharacter->SetEscapeToolNum(MyCharacter->GetEscapeToolNum() - 3);
		}
		else if(MyCharacter->GetEscapeToolNum()>0)
		{
			DamageInsigator->SetEscapeToolNum(DamageInsigator->GetEscapeToolNum() + 1);
			MyCharacter->SetEscapeToolNum(MyCharacter->GetEscapeToolNum() - 1);
		}
	}

	if (MyCharacter && RespawnedController)
	{
		MyCharacter->bAlive = true;
		FName Tagname = TagName;
		AActor* PlayerStarts;
		PlayerStarts=FindPlayerStart(MyCharacter->GetController(), *Tagname.ToString());
		MyCharacter->SetResetState();
	
		MyCharacter->SetActorTransform(PlayerStarts->GetActorTransform());
		//ÆÐÅ¶ BOOL °ª
		if (inst)
			Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Alive_packet(inst->GetPlayerID(), 1);

		//Cast<ACharacterController>(RespawnedController)->ServerDeadSync(MyCharacter->bAlive, inst->GetPlayerID());
		MyCharacter->StopAnimMontage(MyCharacter->GetDeadMontage());
		Cast<ACharacterController>(RespawnedController)->OnPossess(MyCharacter);
		//RestartPlayerAtPlayerStart(RespawnedController, PlayerStarts);
	}
	if (inst)
		inst->m_Socket->Send_Dissolve_packet(inst->GetPlayerID(), 1);
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
	bStarted = true;
	inst->m_Socket->bAllReady = false;
}
