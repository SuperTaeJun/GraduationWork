
// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CharacterController.h"
#include "HUD/MainHUD.h"
#include "HUD/CharacterUi.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/CharacterBase.h"
#include "Character/Character1.h"
#include "Character/Character2.h"
#include "Character/Character3.h"
#include "Character/Character4.h"

#include "Components/Image.h"
#include "Game/BOGameInstance.h"
#include "Network/PacketData.h"
#include <string>
#include "ClientSocket.h"

ACharacterController::ACharacterController()
{
	//c_socket = ClientSocket::GetSingleton();

	p_cnt = -1;
	bNewPlayerEntered = false;
	bNewWeaponEntered = false;
	bInitPlayerSetting = false;
	PrimaryActorTick.bCanEverTick = true;
	Set_Weapon = false;
}

void ACharacterController::BeginPlay()
{


	FInputModeGameOnly GameOnlyInput;
	SetInputMode(GameOnlyInput);

	MainHUD = Cast<AMainHUD>(GetHUD());
	c_socket = ClientSocket::GetSingleton();
	c_socket->SetPlayerController(this);
	////아아 여기
	/*c_socket = new ClientSocket();
	c_socket->SetPlayerController(this);*/
	c_socket->StartListen();
	connect = c_socket->Connect("192.168.101.241", 7777);

	if (connect)
	{
		
		c_socket->StartListen();
		UE_LOG(LogClass, Warning, TEXT("IOCP Server connect success!"));
		FString c_id = "testuser";
		FString c_pw = "1234";
		switch (Cast<UBOGameInstance>(GetGameInstance())->GetCharacterType())
		{
		case ECharacterType::ECharacter1:

			c_socket->Send_Login_Info(TCHAR_TO_UTF8(*c_id), TCHAR_TO_UTF8(*c_pw), PlayerType::Character1);
			break;
		case ECharacterType::ECharacter2:
			c_socket->Send_Login_Info(TCHAR_TO_UTF8(*c_id), TCHAR_TO_UTF8(*c_pw), PlayerType::Character2);
			break;
		case ECharacterType::ECharacter3:
			c_socket->Send_Login_Info(TCHAR_TO_UTF8(*c_id), TCHAR_TO_UTF8(*c_pw), PlayerType::Character3);
			break;
		case ECharacterType::ECharacter4:
			c_socket->Send_Login_Info(TCHAR_TO_UTF8(*c_id), TCHAR_TO_UTF8(*c_pw), PlayerType::Character4);
			break;
		default:
			c_socket->Send_Login_Info(TCHAR_TO_UTF8(*c_id), TCHAR_TO_UTF8(*c_pw), PlayerType::Character1);
			break;
		}

		SleepEx(0.5, true);
	}
	else
	{
		UE_LOG(LogClass, Warning, TEXT("IOCP Server connect FAIL!"));
	}
	SleepEx(0.5, true);
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

void ACharacterController::SetHUDBojoImage(EBojoMugiType Type)
{
	if (MainHUD)
	{
		switch (Type)
		{
		case EBojoMugiType::E_Grenade:
			MainHUD->CharacterUi->BojomugiImage->SetBrushFromTexture(MainHUD->CharacterUi->GrenadeImage);
			break;
		case EBojoMugiType::E_Wall:
			MainHUD->CharacterUi->BojomugiImage->SetBrushFromTexture(MainHUD->CharacterUi->WallImage);
			break;
		case EBojoMugiType::E_BoobyTrap:
			MainHUD->CharacterUi->BojomugiImage->SetBrushFromTexture(MainHUD->CharacterUi->BoobtTrapImage);
			break;
		default:
			MainHUD->CharacterUi->BojomugiImage->SetBrushFromTexture(MainHUD->CharacterUi->GrenadeImage);
			break;
		}
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
	if (MainHUD)
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

void ACharacterController::ShowRespawnSelect()
{
	if (MainHUD)
	{
		MainHUD->AddSelectRespawn();
	}
}

void ACharacterController::InitPlayer()
{
	auto my_player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));
	my_player->SetActorLocationAndRotation(FVector(initplayer.X, initplayer.Y, initplayer.Z), FRotator(0.0f, initplayer.Yaw, 0.0f));
	my_player->_SessionId = initplayer.Id;
	bInitPlayerSetting = false;
}

void ACharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bInitPlayerSetting)
		InitPlayer();
	//새 플레이어 스폰
	if (bNewPlayerEntered)
		UpdateSyncPlayer();

	UpdateWorld();
	//UE_LOG(LogTemp, Warning, TEXT("HHHHHH : %s"), *GetOwner()->GetVelocity().ToString());
	UpdatePlayer();
	SleepEx(0, true);
}

void ACharacterController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	c_socket->CloseSocket();
	c_socket->StopListen();
}


void ACharacterController::SetInitPlayerInfo(const CPlayer& owner_player)
{
	UE_LOG(LogClass, Warning, TEXT("SetInitPlayerInfo"));
	initplayer = owner_player;
	bInitPlayerSetting = true;
	//Set_Weapon = true;
}
void ACharacterController::SetNewCharacterInfo(std::shared_ptr<CPlayer> InitPlayer)
{
	if (InitPlayer != nullptr) {
		bNewPlayerEntered = true;
		NewPlayer.push(InitPlayer);
		UE_LOG(LogTemp, Warning, TEXT("The value of size_: %d"), NewPlayer.size());
	}
}
void ACharacterController::SetNewWeaponMesh(std::shared_ptr<CPlayer> InitPlayer)
{
	if (InitPlayer != nullptr) {
		bNewWeaponEntered = true;
		NewPlayer.push(InitPlayer);

	}
}

bool ACharacterController::UpdateWorld()
{
	UWorld* const world = GetWorld();
	if (world == nullptr)
		return false;
	if (PlayerInfo == nullptr) return false;
	if (PlayerInfo->players.size() == 1)
	{
		return false;
	}
	TArray<AActor*> SpawnPlayer;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterBase::StaticClass(), SpawnPlayer);
	//UE_LOG(LogTemp, Warning, TEXT("Before loop"));
	if (p_cnt == -1)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Inside loop"));
		p_cnt = PlayerInfo->players.size();
		UE_LOG(LogTemp, Warning, TEXT("The value of size_: %d"), p_cnt);
		return false;
	}
	else
	{
		for (auto& player : SpawnPlayer)
		{
			ACharacterBase* OtherPlayer = Cast<ACharacterBase>(player);
			//UE_LOG(LogTemp, Warning, TEXT("Updating player info for ID %d"), OtherPlayer->p_id);
			if (!OtherPlayer || OtherPlayer->_SessionId == -1 || OtherPlayer->_SessionId == id)
			{
				continue;
			}
			CPlayer* info = &PlayerInfo->players[OtherPlayer->_SessionId];


			if (info->IsAlive)
			{
				FVector PlayerLocation;
				PlayerLocation.X = info->X;
				PlayerLocation.Y = info->Y;
				PlayerLocation.Z = info->Z;

				FRotator PlayerRotation;
				PlayerRotation.Yaw = info->Yaw;
				PlayerRotation.Pitch = 0.0f;
				PlayerRotation.Roll = 0.0f;

				//속도
				FVector PlayerVelocity;
				PlayerVelocity.X = info->VeloX;
				PlayerVelocity.Y = info->VeloY;
				PlayerVelocity.Z = info->VeloZ;
				if (!OtherPlayer->GetCurWeapon())
				{
					if (info->w_type == WeaponType::RIFLE)
					{
						FName RifleSocketName = FName("RifleSocket");
						OtherPlayer->SetWeapon(Rifle, RifleSocketName);
					}
					else if (info->w_type == WeaponType::SHOTGUN)
					{
						FName ShotgunSocketName = FName("ShotgunSocket");
						OtherPlayer->SetWeapon(ShotGun, ShotgunSocketName);
					}
					else if (info->w_type == WeaponType::LAUNCHER)
					{
						FName LancherSocketName = FName("LancherSocket");
						OtherPlayer->SetWeapon(Lancher, LancherSocketName);
					}
				}

				OtherPlayer->AddMovementInput(PlayerVelocity);
				OtherPlayer->SetActorRotation(PlayerRotation);
				OtherPlayer->SetActorLocation(PlayerLocation);
				OtherPlayer->GetCharacterMovement()->MaxWalkSpeed = info->Max_Speed;

			}
			else {

			}

		}
	}
	return true;
}
void ACharacterController::UpdateSyncPlayer()
{
	// 동기화 용
	UWorld* const world = GetWorld();
	/*if (other_session_id == id)
		return;*/
	int size_ = NewPlayer.size();
	for (int i = 0; i < size_; ++i)
	{
		UE_LOG(LogTemp, Warning, TEXT("%d"), NewPlayer.front()->Id);
		if (NewPlayer.front()->Id == id)
		{
			UE_LOG(LogTemp, Warning, TEXT("%d %d"), NewPlayer.front()->Id, id);
			NewPlayer.front() = nullptr;
			NewPlayer.pop();
			continue;
		}
		switch (NewPlayer.front()->p_type)
		{
		case PlayerType::Character1:
			if (SkMeshAsset1) {
				FVector S_LOCATION;
				S_LOCATION.X = NewPlayer.front()->X;
				S_LOCATION.Y = NewPlayer.front()->Y;
				S_LOCATION.Z = NewPlayer.front()->Z;
				FRotator S_ROTATOR;
				S_ROTATOR.Yaw = NewPlayer.front()->Yaw;
				S_ROTATOR.Pitch = 0.0f;
				S_ROTATOR.Roll = 0.0f;
				FActorSpawnParameters SpawnActor;
				SpawnActor.Owner = this;
				SpawnActor.Instigator = GetInstigator();
				SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				ToSpawn = ACharacter1::StaticClass();
				ACharacter1* SpawnCharacter = world->SpawnActor<ACharacter1>(ToSpawn,
					S_LOCATION, S_ROTATOR, SpawnActor);
				SpawnCharacter->SpawnDefaultController();
				SpawnCharacter->_SessionId = NewPlayer.front()->Id;
				SpawnCharacter->GetMesh()->SetSkeletalMesh(SkMeshAsset1);
				if (Anim1)
					SpawnCharacter->GetMesh()->SetAnimClass(Anim1);
			}
			break;
		case  PlayerType::Character2:
			if (SkMeshAsset2) {
				FVector S_LOCATION;
				S_LOCATION.X = NewPlayer.front()->X;
				S_LOCATION.Y = NewPlayer.front()->Y;
				S_LOCATION.Z = NewPlayer.front()->Z;
				FRotator S_ROTATOR;
				S_ROTATOR.Yaw = NewPlayer.front()->Yaw;
				S_ROTATOR.Pitch = 0.0f;
				S_ROTATOR.Roll = 0.0f;
				FActorSpawnParameters SpawnActor;
				SpawnActor.Owner = this;
				SpawnActor.Instigator = GetInstigator();
				SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				ToSpawn = ACharacter2::StaticClass();
				ACharacter2* SpawnCharacter = world->SpawnActor<ACharacter2>(ToSpawn,
					S_LOCATION, S_ROTATOR, SpawnActor);
				SpawnCharacter->SpawnDefaultController();
				SpawnCharacter->_SessionId = NewPlayer.front()->Id;
				SpawnCharacter->GetMesh()->SetSkeletalMesh(SkMeshAsset2);
				if (Anim2)
					SpawnCharacter->GetMesh()->SetAnimClass(Anim2);
			}
			break;
		case  PlayerType::Character3:
			if (SkMeshAsset3) {
				FVector S_LOCATION;
				S_LOCATION.X = NewPlayer.front()->X;
				S_LOCATION.Y = NewPlayer.front()->Y;
				S_LOCATION.Z = NewPlayer.front()->Z;
				FRotator S_ROTATOR;
				S_ROTATOR.Yaw = NewPlayer.front()->Yaw;
				S_ROTATOR.Pitch = 0.0f;
				S_ROTATOR.Roll = 0.0f;
				FActorSpawnParameters SpawnActor;
				SpawnActor.Owner = this;
				SpawnActor.Instigator = GetInstigator();
				SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				ToSpawn = ACharacter3::StaticClass();
				ACharacter3* SpawnCharacter = world->SpawnActor<ACharacter3>(ToSpawn,
					S_LOCATION, S_ROTATOR, SpawnActor);
				SpawnCharacter->SpawnDefaultController();
				SpawnCharacter->_SessionId = NewPlayer.front()->Id;
				SpawnCharacter->GetMesh()->SetSkeletalMesh(SkMeshAsset3);
				if (Anim3)
					SpawnCharacter->GetMesh()->SetAnimClass(Anim3);
			}
			break;
		case  PlayerType::Character4:
			if (SkMeshAsset4) {
				FVector S_LOCATION;
				S_LOCATION.X = NewPlayer.front()->X;
				S_LOCATION.Y = NewPlayer.front()->Y;
				S_LOCATION.Z = NewPlayer.front()->Z;
				FRotator S_ROTATOR;
				S_ROTATOR.Yaw = NewPlayer.front()->Yaw;
				S_ROTATOR.Pitch = 0.0f;
				S_ROTATOR.Roll = 0.0f;
				FActorSpawnParameters SpawnActor;
				SpawnActor.Owner = this;
				SpawnActor.Instigator = GetInstigator();
				SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				ToSpawn = ACharacter4::StaticClass();
				ACharacter4* SpawnCharacter = world->SpawnActor<ACharacter4>(ToSpawn,
					S_LOCATION, S_ROTATOR, SpawnActor);
				SpawnCharacter->SpawnDefaultController();
				SpawnCharacter->_SessionId = NewPlayer.front()->Id;
				SpawnCharacter->GetMesh()->SetSkeletalMesh(SkMeshAsset4);
				if (Anim4)
					SpawnCharacter->GetMesh()->SetAnimClass(Anim4);
			}
			break;
		default:
			if (SkMeshAsset1) {
				FVector S_LOCATION;
				S_LOCATION.X = NewPlayer.front()->X;
				S_LOCATION.Y = NewPlayer.front()->Y;
				S_LOCATION.Z = NewPlayer.front()->Z;
				FRotator S_ROTATOR;
				S_ROTATOR.Yaw = NewPlayer.front()->Yaw;
				S_ROTATOR.Pitch = 0.0f;
				S_ROTATOR.Roll = 0.0f;
				FActorSpawnParameters SpawnActor;
				SpawnActor.Owner = this;
				SpawnActor.Instigator = GetInstigator();
				SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				ToSpawn = ACharacter1::StaticClass();
				ACharacter1* SpawnCharacter = world->SpawnActor<ACharacter1>(ToSpawn,
					S_LOCATION, S_ROTATOR, SpawnActor);
				SpawnCharacter->SpawnDefaultController();
				SpawnCharacter->_SessionId = NewPlayer.front()->Id;
				SpawnCharacter->GetMesh()->SetSkeletalMesh(SkMeshAsset1);
				if (Anim1)
					SpawnCharacter->GetMesh()->SetAnimClass(Anim1);
			}
			break;
		}

		if (PlayerInfo != nullptr)
		{
			CPlayer info;
			info.Id = NewPlayer.front()->Id;
			info.X = NewPlayer.front()->X;
			info.Y = NewPlayer.front()->Y;
			info.Z = NewPlayer.front()->Z;

			info.Yaw = NewPlayer.front()->Yaw;

			PlayerInfo->players[NewPlayer.front()->Id] = info;
			p_cnt = PlayerInfo->players.size();
		}

		UE_LOG(LogClass, Warning, TEXT("other spawned player connect"));

		NewPlayer.front() = nullptr;
		NewPlayer.pop();
	}
	bNewPlayerEntered = false;
}
void ACharacterController::UpdateSyncWeapon()
{

}
//void ACharacterController::UpdateSyncPlayer()
//{
//	UWorld* const World = GetWorld();
//	if (!World) return;
//
//	while (!NewPlayer.empty())
//	{
//		auto NewPlayerInfo = NewPlayer.front();
//		if (!NewPlayerInfo) continue;
//
//		if (NewPlayerInfo->Id == id)
//		{
//			// Skip spawning the player if it's the local player
//			NewPlayer.pop();
//			continue;
//		}
//
//		FVector SpawnLocation1;
//		SpawnLocation1.X = NewPlayerInfo->X;
//		SpawnLocation1.Y = NewPlayerInfo->Y;
//		SpawnLocation1.Z = NewPlayerInfo->Z;
//
//		FRotator SpawnRotation;
//		SpawnRotation.Yaw = NewPlayerInfo->Yaw;
//		SpawnRotation.Pitch = 0.0f;
//		SpawnRotation.Roll = 0.0f;
//
//		FActorSpawnParameters SpawnParams;
//		SpawnParams.Owner = this;
//		SpawnParams.Instigator = GetInstigator();
//		SpawnParams.Name = FName(*FString::FromInt(NewPlayerInfo->Id));
//
//		ACharacterBase* SpawnedCharacter = World->SpawnActor<ACharacterBase>(ACharacterBase::StaticClass(),
//			SpawnLocation, SpawnRotation, SpawnParams);
//		if (SpawnedCharacter)
//		{
//			SpawnedCharacter->SpawnDefaultController();
//			SpawnedCharacter->_SessionId = NewPlayerInfo->Id;
//
//			if (PlayerInfo != nullptr)
//			{
//				CPlayer Info;
//				Info.Id = NewPlayerInfo->Id;
//				Info.X = NewPlayerInfo->X;
//				Info.Y = NewPlayerInfo->Y;
//				Info.Z = NewPlayerInfo->Z;
//				Info.Yaw = NewPlayerInfo->Yaw;
//
//				PlayerInfo->players[NewPlayerInfo->Id] = Info;
//				p_cnt = PlayerInfo->players.size();
//			}
//
//			UE_LOG(LogClass, Warning, TEXT("Another player spawned"));
//		}
//
//		NewPlayer.pop(); // Remove processed entry from the queue
//	}
//
//	bNewPlayerEntered = false;
//}

void ACharacterController::UpdatePlayer()
{
	auto m_Player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));
	//my_session_id = m_Player->_SessionId;
	auto MyLocation = m_Player->GetActorLocation();
	auto MyRotation = m_Player->GetActorRotation();
	auto MyVelocity = m_Player->GetVelocity();
	auto max_speed = m_Player->GetCharacterMovement()->MaxWalkSpeed;
	FVector MyCameraLocation;
	FRotator MyCameraRotation;
	m_Player->GetActorEyesViewPoint(MyCameraLocation, MyCameraRotation);
	c_socket->Send_Move_Packet(id, MyLocation, MyRotation, MyVelocity, max_speed);
	//UE_LOG(LogClass, Warning, TEXT("send move packet"));
}

void ACharacterController::Set_Weapon_Type(EWeaponType Type)
{
	switch (Type)
	{
	case EWeaponType::E_Rifle:
		c_socket->Send_Weapon_Type(WeaponType::RIFLE, id);
		break;
	case EWeaponType::E_Shotgun:
		c_socket->Send_Weapon_Type(WeaponType::SHOTGUN, id);
		break;
	case EWeaponType::E_Launcher:
		c_socket->Send_Weapon_Type(WeaponType::LAUNCHER, id);
		break;
	default:
		break;
	}
}

void ACharacterController::UpdateWeaponMesh()
{

}

//pawn 
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
