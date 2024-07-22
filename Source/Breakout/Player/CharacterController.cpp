
#include "Player/CharacterController.h"
#include "HUD/MainHUD.h"
#include "HUD/CharacterUi.h"
#include "HUD/MatchingUi.h"
#include "HUD/EscapeToolNumUi.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/CharacterBase.h"
#include "Character/Character1.h"
#include "Character/Character2.h"
#include "Character/Character3.h"
#include "Character/Character4.h"
#include "Weapon/WeaponBase.h"
#include "Components/Image.h"
#include "Game/BOGameInstance.h"
#include "Weapon/RocketLauncher.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Weapon/ProjectileBase.h"
#include "Weapon/ProjectileBullet.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraActor.h"
#include "TimerManager.h"
#include "FX/Skill4Actor.h"
#include "GameProp/EscapeTool.h"
#include "../../Server/Server/protocol.h"
#include <string>
#include "ClientSocket.h"
#include"Animatiom/BOAnimInstance.h"
#include "Game/BOGameMode.h"
#include "GameProp/BulletHoleWall.h"
#include "Components/SpotLightComponent.h"

ACharacterController::ACharacterController()
{
	//c_socket = ClientSocket::GetSingleton();
	login_cond = false;
	p_cnt = -1;
	bNewPlayerEntered = false;
	bNewWeaponEntered = false;
	bInitPlayerSetting = false;
	PrimaryActorTick.bCanEverTick = true;
	Set_Weapon = false;

}

void ACharacterController::BeginPlay()
{
	//FInputModeGameOnly GameOnlyInput;
	//SetInputMode(GameOnlyInput);
	MainHUD = Cast<AMainHUD>(GetHUD());
	//inst = Cast<UBOGameInstance>(GetGameInstance());
	m_GameMode = Cast<ABOGameMode>(GetWorld()->GetAuthGameMode());

	inst = Cast<UBOGameInstance>(GetGameInstance());
	inst->m_Socket->SetPlayerController(this);
	UE_LOG(LogTemp, Warning, TEXT("BEGIN"));
	if (inst)
	{
		id = inst->GetPlayerID();
		switch (Cast<UBOGameInstance>(GetGameInstance())->GetCharacterType())
		{
		case ECharacterType::ECharacter1:

			inst->m_Socket->Send_Character_Type(PlayerType::Character1, id);
			break;
		case ECharacterType::ECharacter2:
			inst->m_Socket->Send_Character_Type(PlayerType::Character2, id);
			break;
		case ECharacterType::ECharacter3:
			inst->m_Socket->Send_Character_Type(PlayerType::Character3, id);
			break;
		case ECharacterType::ECharacter4:
			inst->m_Socket->Send_Character_Type(PlayerType::Character4, id);
			break;
		default:
			inst->m_Socket->Send_Character_Type(PlayerType::Character1, id);
			break;
		}
	}


	TArray<AActor*> TempActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), WallClass, TempActors);
	for (int i = 0; i< TempActors.Num();++i)
	{
		ABulletHoleWall* BulletWall = Cast<ABulletHoleWall>(TempActors[i]);
		if (BulletWall)
		{
			BulletWalls.Add(BulletWall);
			BulletWalls[i]->ID = i;
		}
	}
	TempActors.Empty();
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEscapeTool::StaticClass(), TempActors);
	for (int i = 0; i < TempActors.Num(); i++)
	{
		AEscapeTool* EscapeWall = Cast<AEscapeTool>(TempActors[i]);
		if (EscapeWall)
		{
			EscapeTools.Add(EscapeWall);
			EscapeTools[i]->ItemID = i;
		}
	}

}

void ACharacterController::SetChName()
{
	if (MainHUD)
	{
		/*if (inst->m_Socket->tempid == 0)
		{*/
		if (inst->m_Socket->Tempname.size() > 0)
		{
			FString Player1 = inst->m_Socket->Tempname.front();
			MainHUD->EscapeToolNumUi->Player1Ch->SetText(FText::FromString(Player1));
			FString Player2 = inst->m_Socket->Tempname.back();
			MainHUD->EscapeToolNumUi->Player2Ch->SetText(FText::FromString(Player2));
		}
	/*	}*/
		/*else if (inst->m_Socket->tempid == 1)
		{
			FString Player1 = inst->m_Socket->TempName;
			MainHUD->EscapeToolNumUi->Player1Ch->SetText(FText::FromString(Player1));
			FString Player2 = inst->m_Socket->TempName2;
			MainHUD->EscapeToolNumUi->Player2Ch->SetText(FText::FromString(Player2));
		}*/
	}
}

//탈출도구
void ACharacterController::SetNum()
{
	if (MainHUD)
	{
		if (MainHUD->EscapeToolNumUi->Player1Ch->Text.ToString() == inst->m_Socket->TempPlayerName) {
			FString Player1 = FString::Printf(TEXT("%d"), inst->m_Socket->Tempcnt2);
			MainHUD->EscapeToolNumUi->Player1->SetText(FText::FromString(Player1));
		}
		else if (MainHUD->EscapeToolNumUi->Player2Ch->Text.ToString() == inst->m_Socket->TempPlayerName) {
			FString Player2 = FString::Printf(TEXT("%d"), inst->m_Socket->Tempcnt2);
			MainHUD->EscapeToolNumUi->Player2->SetText(FText::FromString(Player2));
		}
	}
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

void ACharacterController::SetHUDMatchingUi()
{
	if (MainHUD)
	{
		MainHUD->MatchingUi->ContingText->SetVisibility(ESlateVisibility::Hidden);
		MainHUD->MatchingUi->WaitingText->SetText(FText::FromString("Waiting for OtherPlayer"));
	}
}

void ACharacterController::SetHUDMatchingUi(float Time)
{
	if (MainHUD)
	{
		MainHUD->MatchingUi->WaitingText->SetVisibility(ESlateVisibility::Hidden);
		MainHUD->MatchingUi->ContingText->SetVisibility(ESlateVisibility::Visible);
		FString CountText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Time) + 1);
		MainHUD->MatchingUi->ContingText->SetText(FText::FromString(CountText));
	}
}

void ACharacterController::showWeaponSelect()
{
	if (MainHUD)
	{
		MainHUD->AddSelectWeapon();
		//패킷 무기패킷 다시 보내기
	}
}

void ACharacterController::ShowRespawnSelect()
{
	if (MainHUD)
	{
		MainHUD->AddSelectRespawn();
	}
}

void ACharacterController::ShowMatchingUi()
{
	if (MainHUD)
	{
		MainHUD->AddMatchingUi();
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
	//RecvPacket();
	if (bInitPlayerSetting)
		InitPlayer();

	//새 플레이어 스폰
	if (bNewPlayerEntered)
		UpdateSyncPlayer();
	//if(NewItem.size() == 1)
	//	UpdateSyncItem();

	UpdateWorld();
	//UE_LOG(LogTemp, Warning, TEXT("HHHHHH : %s"), *GetOwner()->GetVelocity().ToString());
	UpdatePlayer();
	//SleepEx(0, true);
	ACharacterBase* BaseCharacter = Cast<ACharacterBase>(GetPawn());
	if (BaseCharacter)
	{
		//UE_LOG(LogClass, Warning, TEXT("hp : %f"), DamagedHp);
		//BaseCharacter->SetHealth(DamgeHp);
		//UE_LOG(LogTemp, Warning, TEXT("my health : %f"), BaseCharacter->GetHealth());
		UGameplayStatics::ApplyDamage(
			GetOwner(),
			damaged,
			this,
			this,
			UDamageType::StaticClass()
		);

		damaged = 0;

	}
	if (MainHUD && inst->m_Socket->bAcquire) {
		SetNum();
		inst->m_Socket->bAcquire = false;
	}
	/*if (MainHUD && inst->m_Socket->bName) {
		SetChName();
	}*/

	if (MainHUD && inst->m_Socket->itemflag) {
		BaseCharacter->SetEscapeToolNum(inst->m_Socket->MyItemCount);
		BaseCharacter->UpdateObtainedEscapeTool();
		inst->m_Socket->itemflag = false;
	}
}

void ACharacterController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/*c_socket->CloseSocket();
	c_socket->StopListen();*/
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

void ACharacterController::SetNewItemInfo(std::shared_ptr<CItem> initItem)
{
	
	UE_LOG(LogTemp, Warning, TEXT("initSetNewItemInfo"));
	NewItem.push(initItem);
}

void ACharacterController::SetAttack(int _id)
{
	UWorld* World = GetWorld();
	PlayerInfo->players[_id].fired = true;

}
void ACharacterController::SetHitEffect(int _id)
{

	UWorld* World = GetWorld();
	PlayerInfo->players[_id].hiteffect = true;
	UE_LOG(LogTemp, Warning, TEXT("ADADADAZVV"));
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

			CPlayer* info = &PlayerInfo->players[OtherPlayer->_SessionId];

			if (!info->IsAlive) continue;

			/*	if (OtherPlayer && OtherPlayer->_SessionId == id)
				{*/
			if (info->bEndGame == true)
			{
				//inst->m_Socket->Exit();
				//Cast<ACharacterBase>(GetPawn())->PlayAnimMontage(Cast<ACharacterBase>(GetPawn())->GetDeadMontage());
				//FGenericPlatformMisc::RequestExit(true);
				GetWorld()->ServerTravel(FString("/Game/Maps/GameRoom"), false, true);
			}
			//}

			if (!OtherPlayer || OtherPlayer->_SessionId == -1 || OtherPlayer->_SessionId == id)
			{
				continue;
			}
			FVector PlayerLocation;
			PlayerLocation.X = info->X;
			PlayerLocation.Y = info->Y;
			PlayerLocation.Z = info->Z;

			FRotator PlayerRotation;
			PlayerRotation.Yaw = info->Yaw;
			PlayerRotation.Pitch = 0.0f;
			PlayerRotation.Roll = 0.0f;

			float AO_YAW = info->AO_YAW;
			float AO_PITCH = info->AO_PITCH;
			//속도
			FVector PlayerVelocity;
			PlayerVelocity.X = info->VeloX;
			PlayerVelocity.Y = info->VeloY;
			PlayerVelocity.Z = info->VeloZ;
			// 나이아가라 레이저
			FVector Firegun;
			FRotator EFiregun;
			Firegun.X = info->Sshot.X;
			Firegun.Y = info->Sshot.Y;
			Firegun.Z = info->Sshot.Z;
			//-----------------------
			// 1번 캐릭터 나이아가라 벡터
			FVector ch1skill;
			FVector ch4skill;
			//------------------------
			//히팅 이팩트
			FVector HEloc;
			HEloc.X = info->Hshot.X;
			HEloc.Y = info->Hshot.Y;
			HEloc.Z = info->Hshot.Z;

			FRotator EffectRot;
			EffectRot.Pitch = info->FEffect.Pitch;
			EffectRot.Yaw = info->FEffect.Yaw;
			EffectRot.Roll = info->FEffect.Roll;

			float SyncHP = info->hp;
			//------------------------
			// 벽 수류탄 관련 
			FVector bulletWallLoc;
			bulletWallLoc.X = info->BulletLoc.X;
			bulletWallLoc.Y = info->BulletLoc.Y;
			bulletWallLoc.Z = info->BulletLoc.Z;

			FRotator bulletWallRot;
			bulletWallRot.Yaw = info->BulletRot.Yaw;
			bulletWallRot.Pitch = info->BulletRot.Pitch;
			bulletWallRot.Roll = info->BulletRot.Roll;
			if (info->bBulletWall)
			{
				for (int i = 0; i < BulletWalls.Num(); i++)
				{
					if (BulletWalls[i]->bUsing == false)
					{
						BulletWalls[i]->bUsing = true;
						BulletWalls[i]->SetActorLocationAndRotation(bulletWallLoc, bulletWallRot);
						break;
					}
				}
				info->bBulletWall = false;
			}
			//------------------------
			if (!OtherPlayer->GetCurWeapon() && info->bselectweapon)
			{

				UE_LOG(LogTemp, Warning, TEXT("WEAPON : %d"), info->w_type);
				if (info->w_type == WeaponType::RIFLE)
				{
					FName RifleSocketName = FName("RifleSocket");
					OtherPlayer->SetWeapon(Rifle, RifleSocketName);
					//UE_LOG(LogTemp, Warning, TEXT("RifleSocket"));

				}
				else if (info->w_type == WeaponType::SHOTGUN)
				{
					UE_LOG(LogTemp, Warning, TEXT("SHOTGUN"));
					FName ShotgunSocketName = FName("ShotgunSocket");
					OtherPlayer->SetWeapon(ShotGun, ShotgunSocketName);

				}
				else if (info->w_type == WeaponType::LAUNCHER)
				{
					FName LancherSocketName = FName("LancherSocket");
					OtherPlayer->SetWeapon(Lancher, LancherSocketName);

				}
				else
				{
					FName LancherSocketName = FName("LancherSocket");
					OtherPlayer->SetWeapon(Lancher, LancherSocketName);

				}
				info->bselectweapon = false;
			}

			OtherPlayer->AddMovementInput(PlayerVelocity);
			OtherPlayer->SetActorRotation(PlayerRotation);
			OtherPlayer->SetActorLocation(PlayerLocation);
			OtherPlayer->SetAO_PITCH(AO_PITCH);
			OtherPlayer->SetAO_YAW(AO_YAW);
			OtherPlayer->GetCharacterMovement()->MaxWalkSpeed = info->Max_Speed;
			
			OtherPlayer->SetHealth(SyncHP);
			OtherPlayer->bAlive = info->bAlive;

			if (info->bGetWeapon == true)
			{
				OtherPlayer->CurWeapon->Destroy();
				OtherPlayer->CurWeapon = nullptr;
				info->bGetWeapon = false;
			}

			//히팅
			if (OtherPlayer->GetCurWeapon() && info->hiteffect == true)
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Owner = OtherPlayer;
				SpawnParameters.Instigator = OtherPlayer;
				if (info->weptype == 0) {
					GetWorld()->SpawnActor<AProjectileBullet>(BulletRef, HEloc, EffectRot, SpawnParameters);
					info->hiteffect = false;
				}
				else if (info->weptype == 1) {
					GetWorld()->SpawnActor<AProjectileBase>(LauncherRef, HEloc, EffectRot, SpawnParameters);
					info->hiteffect = false;
				}
			}
			if (info->bojotype != 3)
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Owner = OtherPlayer;
				SpawnParameters.Instigator = OtherPlayer;
				if (info->bojotype == 0) {
					GetWorld()->SpawnActor<AProjectileBase>(GrenadeRef, HEloc, EffectRot, SpawnParameters);
					UE_LOG(LogTemp, Warning, TEXT("FIRE"));
					info->bojotype = 3;
				}
				else if (info->bojotype == 1) {
					GetWorld()->SpawnActor<AProjectileBase>(WallRef, HEloc, EffectRot, SpawnParameters);
					UE_LOG(LogTemp, Warning, TEXT("WALL"));
					info->bojotype = 3;
				}
				else if (info->bojotype == 2) {
					GetWorld()->SpawnActor<AProjectileBase>(BoobyTrapRef, HEloc, EffectRot, SpawnParameters);
					info->bojotype = 3;
				}
			}
			if (info->bojoanimtype == 0)
			{
				OtherPlayer->PlayAnimMontage(GrenadeMontage, 1.5f);
				info->bojoanimtype = 1;
			}
			FVector Vshotgun;
			FRotator Rshotgun;
			FRotator Rshotgun1;
			FRotator Rshotgun2;
			FRotator Rshotgun3;
			FRotator Rshotgun4;
			FRotator Rshotgun5;
			FRotator Rshotgun6;
			FRotator Rshotgun7;
			FRotator Rshotgun8;
			Vshotgun.X = info->sSshot.X;
			Vshotgun.Y = info->sSshot.Y;
			Vshotgun.Z = info->sSshot.Z;
			//--------------------------
			Rshotgun.Pitch = info->sEshot.Pitch;
			Rshotgun.Yaw = info->sEshot.Yaw;
			Rshotgun.Roll = info->sEshot.Roll;
			Rshotgun1.Pitch = info->sEshot1.Pitch;
			Rshotgun1.Yaw = info->sEshot1.Yaw;
			Rshotgun1.Roll = info->sEshot1.Roll;
			Rshotgun2.Pitch = info->sEshot2.Pitch;
			Rshotgun2.Yaw = info->sEshot2.Yaw;
			Rshotgun2.Roll = info->sEshot2.Roll;
			Rshotgun3.Pitch = info->sEshot3.Pitch;
			Rshotgun3.Yaw = info->sEshot3.Yaw;
			Rshotgun3.Roll = info->sEshot3.Roll;
			Rshotgun4.Pitch = info->sEshot4.Pitch;
			Rshotgun4.Yaw = info->sEshot4.Yaw;
			Rshotgun4.Roll = info->sEshot4.Roll;
			//----------------------------------------
			if (OtherPlayer->GetCurWeapon() && info->sfired == true)
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Owner = OtherPlayer;
				SpawnParameters.Instigator = OtherPlayer;
				GetWorld()->SpawnActor<AProjectileBullet>(ShotgunRef, Vshotgun, Rshotgun, SpawnParameters);
				GetWorld()->SpawnActor<AProjectileBullet>(ShotgunRef, Vshotgun, Rshotgun1, SpawnParameters);
				GetWorld()->SpawnActor<AProjectileBullet>(ShotgunRef, Vshotgun, Rshotgun2, SpawnParameters);
				GetWorld()->SpawnActor<AProjectileBullet>(ShotgunRef, Vshotgun, Rshotgun3, SpawnParameters);
				GetWorld()->SpawnActor<AProjectileBullet>(ShotgunRef, Vshotgun, Rshotgun4, SpawnParameters);
				info->sfired = false;
			}


			if (info->skilltype == 0 && info->p_type == PlayerType::Character1)
			{
				if (Cast<ACharacter1>(OtherPlayer)) {
					ACharacter1* Niagaraplayer = Cast<ACharacter1>(OtherPlayer);
					Niagaraplayer->GetMesh()->SetVisibility(false);
					Niagaraplayer->GetCurWeapon()->GetWeaponMesh()->SetVisibility(false);
					ch1skill.X = info->CH1NiaLoc.X;
					ch1skill.Y = info->CH1NiaLoc.Y;
					ch1skill.Z = info->CH1NiaLoc.Z;
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TimeReplayNiagaraRef, ch1skill);

					info->skilltype = -1;
				}

			}
			else if (info->p_type == PlayerType::Character1 && info->skilltype == 1) {
				if (Cast<ACharacter1>(OtherPlayer)) {
					ACharacter1* Niagaraplayer = Cast<ACharacter1>(OtherPlayer);
					Niagaraplayer->GetMesh()->SetVisibility(true);
					Niagaraplayer->GetCurWeapon()->GetWeaponMesh()->SetVisibility(true);

					info->skilltype = -1;
				}
			}
			else if (info->p_type == PlayerType::Character2 && info->skilltype == 0) {

				if (Cast<ACharacter2>(OtherPlayer)) {
					ACharacter2* Niagaraplayer = Cast<ACharacter2>(OtherPlayer);
					Niagaraplayer->ServerNiagaraSync();
					info->skilltype = -1;
				}
			}
			else if (info->p_type == PlayerType::Character2 && info->bFinishSkill)
			{
				ACharacter2* Niagaraplayer = Cast<ACharacter2>(OtherPlayer);
				Niagaraplayer->GetMesh()->SetVisibility(true);
				Niagaraplayer->GetCurWeapon()->GetWeaponMesh()->SetVisibility(true);
				info->bFinishSkill = false;
			}
			else if (info->p_type == PlayerType::Character3 && info->skilltype == 0)
			{
				if (Cast<ACharacter3>(OtherPlayer)) {
					ACharacter3* Niagaraplayer = Cast<ACharacter3>(OtherPlayer);
					Niagaraplayer->ServerGhostStart();
					info->skilltype = -1;
				}
			}
			else if (info->p_type == PlayerType::Character3 && info->skilltype == 1) {
				if (Cast<ACharacter3>(OtherPlayer)) {
					ACharacter3* Niagaraplayer = Cast<ACharacter3>(OtherPlayer);
					Niagaraplayer->ServerGhostEnd();
					info->skilltype = -1;
				}
			}
			else if (info->skilltype == 0 && info->p_type == PlayerType::Character4)
			{
				if (ACharacter4* Niagaraplayer = Cast<ACharacter4>(OtherPlayer)) {
					Niagaraplayer->SaveCurLocation();
					FActorSpawnParameters SpawnParameters;
					SpawnParameters.Owner = OtherPlayer;
					SpawnParameters.Instigator = OtherPlayer;
					ch4skill.X = info->CH1NiaLoc.X;
					ch4skill.Y = info->CH1NiaLoc.Y;
					ch4skill.Z = info->CH1NiaLoc.Z;
					ServerTemp = GetWorld()->SpawnActor<ANiagaraActor>(NiagaraActorRef, ch4skill, FRotator::ZeroRotator, SpawnParameters);
					info->skilltype = -1;
				}

			}
			else if (info->p_type == PlayerType::Character4 && info->skilltype == 1) {
				if (Cast<ACharacter4>(OtherPlayer)) {
					ACharacter4* Niagaraplayer = Cast<ACharacter4>(OtherPlayer);
					Niagaraplayer->GetNiagaraComp()->Activate();
					Niagaraplayer->GetMesh()->SetVisibility(false, false);
					Niagaraplayer->GetCurWeapon()->GetWeaponMesh()->SetVisibility(false);
					info->skilltype = -1;
					//Cast<ASkill4Actor>(ServerTemp)->bTimerStart = true;
				}
			}
			if (info->p_type == PlayerType::Character4 && info->skilltype == 2) {
				ACharacter4* Niagaraplayer = Cast<ACharacter4>(OtherPlayer);
				Niagaraplayer->GetNiagaraComp()->Deactivate();
				Niagaraplayer->GetMesh()->SetVisibility(true, false);
				Niagaraplayer->GetCurWeapon()->GetWeaponMesh()->SetVisibility(true);
				info->bch4end = false;
				bool Test = Niagaraplayer->GetMesh()->IsVisible();
				UE_LOG(LogTemp, Warning, TEXT("hahah : %d"), Test);
				info->skilltype = -1;
				if (ServerTemp)
					ServerTemp->Destroy();
			}
			// 죽는 애니메이션
			if (info->deadtype == 1) { //처리
				OtherPlayer->StopAnimMontage(SyncDeadMontage);
				info->deadtype = 2;
				OtherPlayer->SetHealth(100.f);
				ServerSetDissolve(false, OtherPlayer);
			}
			else if (info->deadtype == 0) {
				OtherPlayer->PlayAnimMontage(SyncDeadMontage);
				info->deadtype = 2;
				ServerSetDissolve(true, OtherPlayer);
				//info->dissolve = 2;
			}
			// 죽을 때 디졸브
			
			if (info->bServerReload == true)
			{
				OtherPlayer->PlayAnimMontage(SyncReloadMontageCh3);
				info->bServerReload = false;
			}
			if (info->itemAnimtype == 0)
			{
				UAnimInstance* AnimInstance = OtherPlayer->GetMesh()->GetAnimInstance();
				Cast<UBOAnimInstance>(AnimInstance)->bUseLeftHand = false;
				OtherPlayer->PlayAnimMontage(SyncInterMontage);
				info->itemAnimtype = -1;
			}
			else if (info->itemAnimtype == 1)
			{
				UAnimInstance* AnimInstance = OtherPlayer->GetMesh()->GetAnimInstance();
				Cast<UBOAnimInstance>(AnimInstance)->bUseLeftHand = true;
				OtherPlayer->StopAnimMontage(SyncInterMontage);
				info->itemAnimtype = -1;
			}
			if (OtherPlayer->GetCurWeapon()) {
				if (info->bLightOn == true)
				{
					OtherPlayer->CurWeapon->GetSpotLight()->SetVisibility(true);
				}
				else if (info->bLightOn == false)
				{
					OtherPlayer->CurWeapon->GetSpotLight()->SetVisibility(false);
				}
			}

			if (info->bDestroyItem) {
				for (int i = 0; i < EscapeTools.Num(); i++)
				{
					if (EscapeTools[i])
						if (Escapeid == EscapeTools[i]->ItemID)
						{
							Cast<AEscapeTool>(EscapeTools[i])->Destroy();
							EscapeTools[i] = nullptr;
						}
				}
				info->bDestroyItem = false;
			}
			if (inst->m_Socket->bitemcount == true) {
				OtherPlayer->SetEscapeToolNum(info->itemCount);
				UE_LOG(LogTemp, Warning, TEXT("id : %d, itemcount : %d"), OtherPlayer->_SessionId, info->itemCount);
				inst->m_Socket->bitemcount = false;
			}
			// 모프 동기화
			if (inst->m_Socket->MoppType == 0) {
				//Cast<AEscapeTool>(m_GameMode->EscapeTools[MoppID])->TransformMesh(inst->m_Socket->TempMoppTime, false, false);
				EscapeTools[MoppID]->bOverlap = 0;
				inst->m_Socket->MoppType = -1;
			}
			else if (inst->m_Socket->MoppType == 1) {
				EscapeTools[MoppID]->bOverlap = 1;
				inst->m_Socket->MoppType = -1;
			}
			else if (inst->m_Socket->MoppType == 2) {
				if (EscapeTools[MoppID]) {
					EscapeTools[MoppID]->bOverlap = 2;
					inst->m_Socket->MoppType = -1;
				}
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
				//SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				SpawnActor.Name = FName("char1");
				ToSpawn = ACharacter1::StaticClass();
				if (world) {
					ACharacter1* SpawnCharacter = world->SpawnActor<ACharacter1>(ToSpawn,
						S_LOCATION, S_ROTATOR, SpawnActor);
					if (SpawnCharacter) {
						SpawnCharacter->SpawnDefaultController();
						SpawnCharacter->_SessionId = NewPlayer.front()->Id;
						SpawnCharacter->GetMesh()->SetSkeletalMesh(SkMeshAsset1);
						if (Anim1)
							SpawnCharacter->GetMesh()->SetAnimClass(Anim1);
						SpawnCharacter->SetHealth(100.f);
					}
				}
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
				//SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				SpawnActor.Name = FName(SpawnActor.Name = FName("char2"));
				ToSpawn = ACharacter2::StaticClass();
				if (world)
				{
					ACharacter2* SpawnCharacter = world->SpawnActor<ACharacter2>(ToSpawn,
						FVector::ZeroVector, FRotator::ZeroRotator, SpawnActor);
					if (SpawnCharacter) {
						SpawnCharacter->SpawnDefaultController();
						SpawnCharacter->_SessionId = NewPlayer.front()->Id;
						SpawnCharacter->GetMesh()->SetSkeletalMesh(SkMeshAsset2);
					}
					if (Anim2)
						SpawnCharacter->GetMesh()->SetAnimClass(Anim2);
					SpawnCharacter->SetHealth(100.f);
				}
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
				//SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				SpawnActor.Name = FName(SpawnActor.Name = FName("char3"));
				ToSpawn = ACharacter3::StaticClass();
				if (world) {
					ACharacter3* SpawnCharacter = world->SpawnActor<ACharacter3>(ToSpawn,
						FVector::ZeroVector, FRotator::ZeroRotator, SpawnActor);
					if (SpawnCharacter) {
						SpawnCharacter->SpawnDefaultController();
						SpawnCharacter->_SessionId = NewPlayer.front()->Id;
						SpawnCharacter->GetMesh()->SetSkeletalMesh(SkMeshAsset3);
					}

					DynamicMaterial = UMaterialInstanceDynamic::Create(OldMaterial, this);
					if (DynamicMaterial)
					{
						SpawnCharacter->GetMesh()->SetMaterial(0, DynamicMaterial);
						DynamicMaterial->SetScalarParameterValue(FName("Alpha"), 0.f);
					}
					if (Anim3)
						SpawnCharacter->GetMesh()->SetAnimClass(Anim3);
					SpawnCharacter->SetHealth(100.f);
				}
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
				//SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				SpawnActor.Name = FName(SpawnActor.Name = FName("char4"));
				ToSpawn = ACharacter4::StaticClass();
				if (world)
				{
					ACharacter4* SpawnCharacter = world->SpawnActor<ACharacter4>(ToSpawn,
						FVector::ZeroVector, FRotator::ZeroRotator, SpawnActor);
					if (SpawnCharacter) {
						SpawnCharacter->SpawnDefaultController();
						SpawnCharacter->_SessionId = NewPlayer.front()->Id;
						SpawnCharacter->GetMesh()->SetSkeletalMesh(SkMeshAsset4);
					}
					if (Anim4)
						SpawnCharacter->GetMesh()->SetAnimClass(Anim4);
					SpawnCharacter->SetHealth(100.f);
				}
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
				//SpawnActor.Name = FName(*FString(to_string(NewPlayer.front()->Id).c_str()));
				SpawnActor.Name = FName(SpawnActor.Name = FName("char4"));
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


//void ACharacterController::UpdateSyncItem()
//{
//	UWorld* const world = GetWorld();
//	int size_ = NewItem.size();
//	for (int i = 0; i < size_; ++i)
//	{
//		UE_LOG(LogTemp, Warning, TEXT("UpdateSyncItem"));
//
//		FVector S_LOCATION;
//		S_LOCATION.X = NewItem.front()->X;
//		S_LOCATION.Y = NewItem.front()->Y;
//		S_LOCATION.Z = NewItem.front()->Z;
//		FRotator S_ROTATOR;
//		S_ROTATOR.Yaw = 0.0f;
//		S_ROTATOR.Pitch = 0.0f;
//		S_ROTATOR.Roll = 0.0f;
//		FActorSpawnParameters SpawnActor;
//		SpawnActor.Owner = this;
//		SpawnActor.Instigator = GetInstigator();
//		SpawnActor.Name = FName(*FString(to_string(NewItem.front()->Id).c_str()));
//		AEscapeTool* SpawnCharacter = world->SpawnActor<AEscapeTool>(ItemSpawn,
//			S_LOCATION, S_ROTATOR, SpawnActor);
//		if (ItemInfo != nullptr)
//		{
//			CItem info;
//			info.Id = NewItem.front()->Id;
//			info.X = NewItem.front()->X;
//			info.Y = NewItem.front()->Y;
//			info.Z = NewItem.front()->Z;
//			ItemInfo->items[NewItem.front()->Id] = info;
//		}
//		NewItem.front() = nullptr;
//		NewItem.pop();
//	}
//}

void ACharacterController::UpdatePlayer()
{
	auto m_Player = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));
	//my_session_id = m_Player->_SessionId;
	auto MyLocation = m_Player->GetActorLocation();
	auto MyRotation = m_Player->GetActorRotation();
	auto MyVelocity = m_Player->GetVelocity();
	auto max_speed = m_Player->GetCharacterMovement()->MaxWalkSpeed;
	auto AO_Yaw = m_Player->GetAO_Yaw();
	auto AO_Pitch = m_Player->GetAO_Pitch();
	FVector MyCameraLocation;
	FRotator MyCameraRotation;
	m_Player->GetActorEyesViewPoint(MyCameraLocation, MyCameraRotation);
	inst->m_Socket->Send_Move_Packet(id, MyLocation, MyRotation, MyVelocity, max_speed , AO_Yaw, AO_Pitch);
	//UE_LOG(LogClass, Warning, TEXT("send move packet"));
}

void ACharacterController::Set_Weapon_Type(EWeaponType Type)
{
	switch (Type)
	{
	case EWeaponType::E_Rifle:
		inst->m_Socket->Send_Weapon_Type(WeaponType::RIFLE, id);
		break;
	case EWeaponType::E_Shotgun:
		inst->m_Socket->Send_Weapon_Type(WeaponType::SHOTGUN, id);
		break;
	case EWeaponType::E_Launcher:
		inst->m_Socket->Send_Weapon_Type(WeaponType::LAUNCHER, id);
		break;
	default:
		break;
	}
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

void ACharacterController::SeverHpSync(bool bAlive, float hp, int myid)
{
	if (inst)
		inst->m_Socket->Send_My_HP_PACKET(myid, hp, bAlive);

}

void ACharacterController::ServerSetDissolve(bool dissolve, ACharacterBase* player)
{
	if (dissolve == true)
	{
		player->SetbDissolve(true);
		//player->GetCurWeapon()->SetActorHiddenInGame(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WWWWWWWWW"));
		if (player->GetDynamicMaterial())
		{
			player->SetbDissolve(false);
			player->SetDissolvePersent(-1.f);
			player->GetMesh()->SetMaterial(0, player->GetDynamicMaterial());
			player->GetDynamicMaterial()->SetScalarParameterValue(FName("Dissolve"), player->GetDissolvePersent());
		}
	}
}

void ACharacterController::InitializeTree()
{
	// 트리의 루트 노드 초기화
	RootNode = FTreeNode();


	TMap<FString, RotAndLoc> Locations;
	RotAndLoc Temp(FVector(100, 0, 0), FRotator(0.f, 0.f, 10.f));
	Locations.Add(TEXT("Type1"), Temp);
	Locations.Add(TEXT("Type2"), Temp);
	Locations.Add(TEXT("Type3"), Temp);

	RootNode.Children.Add(FTreeNode(Locations));

}

RotAndLoc ACharacterController::GetRandomLocation(FTreeNode& Node, const FString& ObjectType)
{
	// 현재 노드가 리프 노드인지 확인
	if (Node.Children.Num() == 0)
	{
		// 리프 노드라면, 해당 타입의 위치가 있는지 확인
		if (Node.Transform.Contains(ObjectType))
		{
			RotAndLoc Transform = Node.Transform[ObjectType];
			Node.Transform.Remove(ObjectType);
			return Transform;
		}
		// 해당 타입의 위치가 없는 경우 기본 위치 반환
		return RotAndLoc(FVector::ZeroVector, FRotator::ZeroRotator);
	}

	// 자식 노드 중에서 랜덤하게 선택
	int32 Index = FMath::RandRange(0, Node.Children.Num() - 1);
	FTreeNode& SelectedNode = Node.Children[Index];

	RotAndLoc Transform = GetRandomLocation(SelectedNode, ObjectType);

	// 선택된 노드가 비어 있으면 트리에서 제거
	if (SelectedNode.Transform.Num() == 0 && SelectedNode.Children.Num() == 0)
	{
		Node.Children.RemoveAt(Index);
	}

	return Transform;
}
