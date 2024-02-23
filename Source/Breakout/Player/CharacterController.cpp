// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CharacterController.h"
#include "HUD/MainHUD.h"
#include "HUD/CharacterUi.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/CharacterBase.h"
void ACharacterController::BeginPlay()
{
	FInputModeGameOnly GameOnlyInput;
	SetInputMode(GameOnlyInput);

	MainHUD = Cast<AMainHUD>(GetHUD());
	
}

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

void ACharacterController::showWeaponSelect()
{
	if (MainHUD)
	{
		MainHUD->AddSelectWeapon();
	}
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
	connect_player->Send_Move_Packet(my_session_id, MyLocation.X, MyLocation.Y, MyLocation.Z);
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
	// 액터 생성될 때 사용되는 함수
	// Template: 스폰될 액터의 템플릿 액터를 지정합니다.
	// Owner: 액터의 소유자를 지정합니다.일반적으로 액터를 생성한 액터가 소유자가 됩니다.
	// Instigator : 액터의 유발자를 지정합니다.이는 액터가 어떤 이벤트에 의해 생성되었는지를 나타냅니다.
	// SpawnCollisionHandlingOverride : 액터의 충돌 처리 방식을 지정합니다.
	// bNoFail : 액터 생성에 실패할 경우 오류를 발생시키지 않도록 지정합니다.
	// bDeferConstruction : 액터의 구성 요소의 구성을 지연시킵니다.이렇게 하면 
	// 구성 요소의 속성을 설정한 다음 액터를 구축할 수 있습니다.
	// Name : 액터의 이름을 지정합니다.
	// --------------------------
	FActorSpawnParameters SpawnActor;
	ACharacterBase* SpawnCharacter = world->SpawnActor<ACharacterBase>(ToSpawn, 
		S_LOCATION, FRotator::ZeroRotator, SpawnActor);
	SpawnCharacter->SpawnDefaultController();
	SpawnCharacter->_SessionId = other_session_id;

}

void ACharacterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ACharacterBase* BaseCharacter = Cast<ACharacterBase>(InPawn);

	if (BaseCharacter)
	{
		SetHUDHealth(BaseCharacter->GetHealth(), BaseCharacter->MaxGetHealth());
		SetHUDStamina(BaseCharacter->GetStamina(), BaseCharacter->MaxGetStamina());
	}
}
