// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CharacterController.h"
#include "HUD/MainHUD.h"
#include "HUD/CharacterUi.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/CharacterBase.h"
#include "Components/Image.h"
#include "Game/BOGameInstance.h"
void ACharacterController::BeginPlay()
{
	FInputModeGameOnly GameOnlyInput;
	SetInputMode(GameOnlyInput);

	MainHUD = Cast<AMainHUD>(GetHUD());
	
}
//void ACharacterController::OnPossess(APawn* InPawn)
//{
//	Super::OnPossess(InPawn);
//
//	ACharacterBase* Ch = Cast<ACharacterBase>(InPawn);
//	if (Ch)
//	{
//		Ch->SetWeaponUi();
//	}
//
//}
void ACharacterController::SetHUDHealth(float Health, float MaxHealth)
{
	if (MainHUD)
	{
		float HpPercent = Health / MaxHealth;
		MainHUD->CharacterUi->HealthBar->SetPercent(HpPercent);
		MainHUD->CharacterUi->HealthBar->SetFillColorAndOpacity(FLinearColor::Red);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		MainHUD->CharacterUi->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ACharacterController::SetHUDStamina(float Stamina, float MaxStamina)
{
	if (MainHUD)
	{
		float StaminaPercent = Stamina / MaxStamina;
		MainHUD->CharacterUi->StaminaBar->SetPercent(StaminaPercent);
		MainHUD->CharacterUi->StaminaBar->SetFillColorAndOpacity(FLinearColor::Blue);
		FString StaminaText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Stamina), FMath::CeilToInt(MaxStamina));
		MainHUD->CharacterUi->StaminaText->SetText(FText::FromString(StaminaText));
	}
}
void ACharacterController::SetHUDAmmo(int32 Ammo)
{
	if (MainHUD)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		MainHUD->CharacterUi->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ACharacterController::SetHUDEscapeTool(int32 EscapeTool)
{
	if (MainHUD)
	{
		FString EscapeToolText = FString::Printf(TEXT("%d"), EscapeTool);
		MainHUD->CharacterUi->ToolAmount->SetText(FText::FromString(EscapeToolText));
	}
}

void ACharacterController::SetHUDCrosshair(const FCrosshairPackage& Package)
{
	if (MainHUD)
	{
		MainHUD->SetHUDPackage(Package);
	}
}

void ACharacterController::SetHUDSkill()
{
	if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() == ECharacterType::ECharacter1)
	{
		MainHUD->CharacterUi->SkillImage->SetBrushFromTexture(MainHUD->CharacterUi->SkillIcon1);
	}
	else if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() == ECharacterType::ECharacter2)
	{
		MainHUD->CharacterUi->SkillImage->SetBrushFromTexture(MainHUD->CharacterUi->SkillIcon2);
	}
	else if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() == ECharacterType::ECharacter3)
	{
		MainHUD->CharacterUi->SkillImage->SetBrushFromTexture(MainHUD->CharacterUi->SkillIcon3);
	}
	else if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() == ECharacterType::ECharacter4)
	{
		MainHUD->CharacterUi->SkillImage->SetBrushFromTexture(MainHUD->CharacterUi->SkillIcon4);
	}
}

void ACharacterController::SetHUDSkillOpacity(float Opacity)
{
	if(MainHUD)
		MainHUD->CharacterUi->SkillImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, Opacity));
}

void ACharacterController::SetHUDCool(float Cool, float MaxCool)
{
	FString CoolText = FString::Printf(TEXT("%d"), FMath::FloorToInt(MaxCool - Cool));
	if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() != ECharacterType::ECharacter2)
		MainHUD->CharacterUi->SkillCool->SetText(FText::FromString(CoolText));
}

void ACharacterController::SetHUDCool(int32 Cool)
{
	FString CoolText = FString::Printf(TEXT("%d"), Cool);
	if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType() == ECharacterType::ECharacter2)
		MainHUD->CharacterUi->SkillCool->SetText(FText::FromString(CoolText));
}

void ACharacterController::SetHUDCoolVisibility(bool bVisibility)
{
	if (bVisibility)
	{
		MainHUD->CharacterUi->SkillCool->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		MainHUD->CharacterUi->SkillCool->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ACharacterController::showWeaponSelect()
{
	if (MainHUD)
	{
		MainHUD->AddSelectWeapon();
	}
}

void ACharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void ACharacterController::RecvNewPlayer(int sessionID, float x, float y, float z)
{
	bNewPlayerEntered = true;
	other_session_id = sessionID;
	other_x = x;
	other_x = y;
	other_x = z;
}

void ACharacterController::SendPlayerPos(int id)
{
	auto m_Player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));
	my_session_id = m_Player->_SessionId;
	auto MyLocation = m_Player->GetActorLocation();
	auto MyRotation = m_Player->GetActorRotation();
	auto MyVelocity = m_Player->GetVelocity();
	//connect_player->Send_Move_Packet(my_session_id, MyLocation.X, MyLocation.Y, MyLocation.Z);
}

void ACharacterController::SetNewCharacterInfo(std::shared_ptr<CPlayer*> InitPlayer)
{
	if (InitPlayer != nullptr){
		bNewPlayerEntered = true;
		NewPlayer.push(*InitPlayer);
	}
}


void ACharacterController::UpdateSyncPlayer()
{
	// 동기화 용
	UWorld* const world = GetWorld();
	if (other_session_id == my_session_id)
		return;
	FVector S_LOCATION;
	S_LOCATION.X = other_x;
	S_LOCATION.Y = other_y;
	S_LOCATION.Z = other_z;
	FRotator S_ROTATOR;
	S_ROTATOR.Yaw = NewPlayer.front()->Yaw;
	S_ROTATOR.Pitch = 0.0;
	S_ROTATOR.Roll = 0.0;

	FActorSpawnParameters SpawnActor;
	ACharacterBase* SpawnCharacter = world->SpawnActor<ACharacterBase>(ToSpawn, 
		S_LOCATION, FRotator::ZeroRotator, SpawnActor);
	SpawnCharacter->SpawnDefaultController();
	SpawnCharacter->_SessionId = other_session_id;
	if (PlayerInfo != nullptr)
	{
		CPlayer info;
		info.Id = NewPlayer.front()->Id;
		info.X = NewPlayer.front()->X;
		info.Y = NewPlayer.front()->Y;
		info.Z = NewPlayer.front()->Z;

		info.Yaw = NewPlayer.front()->Yaw;

		PlayerInfo->players[NewPlayer.front()->Id] = info;
		count = PlayerInfo->players.size();
	}

	UE_LOG(LogClass, Warning, TEXT("other spawned player connect"));

	NewPlayer.front() = nullptr;
	NewPlayer.pop();
}


void ACharacterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ACharacterBase* BaseCharacter = Cast<ACharacterBase>(InPawn);

	if (BaseCharacter)
	{
		SetHUDHealth(BaseCharacter->GetHealth(), BaseCharacter->MaxGetHealth());
		SetHUDStamina(BaseCharacter->GetStamina(), BaseCharacter->MaxGetStamina());
		FInputModeUIOnly UiGameInput;
		SetInputMode(UiGameInput);
		DisableInput(this);
		bShowMouseCursor = true;
		bEnableMouseOverEvents = true;
		showWeaponSelect();
	}
}
