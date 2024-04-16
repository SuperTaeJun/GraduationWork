
#include "Player/CharacterController.h"
#include "HUD/MainHUD.h"
#include "HUD/CharacterUi.h"
#include "HUD/MatchingUi.h"
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

#include "../../Server/Server/ServerCore/protocol.h"
#include <string>
#include "ClientSocket.h"


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

//void ACharacterController::RecvPacket()
//{
//	//UE_LOG(LogClass, Warning, TEXT("RECVPACKET"));
//
//	int bufferSize = c_socket->buffer.unsafe_size();
//
//	while (remainData < bufferSize)
//	{
//		while (remainData < BUFSIZE && remainData < bufferSize)
//		{
//			c_socket->buffer.try_pop(data[remainData++]);
//		}
//
//		while (remainData > 0 && data[0] <= remainData)
//		{
//			if (c_socket->PacketProcess(data) == false)
//				return;
//			remainData -= data[0];
//			bufferSize -= data[0];
//			memcpy(data, data + data[0], BUFSIZE - data[0]);
//		}
//	}
//
//}

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
		//BaseCharacter->SetHealth(damaged);
		damaged = 0;
		//BaseCharacter->SetHealth(BaseCharacter->GetHealth());
		SetHUDHealth(BaseCharacter->GetHealth(), BaseCharacter->MaxGetHealth());
	}

	//if (inst->m_Socket->bAllReady == true)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("ballready!!!!!!!!!!!!!!!!!"));
	//	DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	//	//GetWorldTimerManager().SetTimer(StartTimeHandle, this, &ABOGameMode::StartGame, 5.f);

	//}
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
			OtherPlayer->GetCharacterMovement()->MaxWalkSpeed = info->Max_Speed;

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
				else if (info->weptype == 2) {
					OtherPlayer->PlayAnimMontage(GrenadeMontage, 1.f, FName("Fire"));
					GetWorld()->SpawnActor<AProjectileBase>(GrenadeRef, HEloc, EffectRot, SpawnParameters);

					info->hiteffect = false;
				}
				else if (info->weptype == 3) {
					OtherPlayer->PlayAnimMontage(GrenadeMontage, 1.f, FName("Fire"));
					GetWorld()->SpawnActor<AProjectileBase>(WallRef, HEloc, EffectRot, SpawnParameters);
					info->hiteffect = false;
				}
				else if (info->weptype == 4) {
					GetWorld()->SpawnActor<AProjectileBase>(BoobyTrapRef, HEloc, EffectRot, SpawnParameters);
					info->hiteffect = false;
				}
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
			Rshotgun5.Pitch = info->sEshot5.Pitch;
			Rshotgun5.Yaw = info->sEshot5.Yaw;
			Rshotgun5.Roll = info->sEshot5.Roll;
			Rshotgun6.Pitch = info->sEshot6.Pitch;
			Rshotgun6.Yaw = info->sEshot6.Yaw;
			Rshotgun6.Roll = info->sEshot6.Roll;
			Rshotgun7.Pitch = info->sEshot7.Pitch;
			Rshotgun7.Yaw = info->sEshot7.Yaw;
			Rshotgun7.Roll = info->sEshot7.Roll;
			Rshotgun8.Pitch = info->sEshot8.Pitch;
			Rshotgun8.Yaw = info->sEshot8.Yaw;
			Rshotgun8.Roll = info->sEshot8.Roll;
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
				GetWorld()->SpawnActor<AProjectileBullet>(ShotgunRef, Vshotgun, Rshotgun5, SpawnParameters);
				GetWorld()->SpawnActor<AProjectileBullet>(ShotgunRef, Vshotgun, Rshotgun6, SpawnParameters);
				GetWorld()->SpawnActor<AProjectileBullet>(ShotgunRef, Vshotgun, Rshotgun7, SpawnParameters);
				GetWorld()->SpawnActor<AProjectileBullet>(ShotgunRef, Vshotgun, Rshotgun8, SpawnParameters);
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
			else if (info->p_type == PlayerType::Character3 && info->skilltype == 0)
			{
				if (Cast<ACharacter3>(OtherPlayer)) {
					ACharacter3* Niagaraplayer = Cast<ACharacter3>(OtherPlayer);
					Niagaraplayer->GetMesh()->SetMaterial(0, Niagaraplayer->DynamicMaterial);
					Niagaraplayer->DynamicMaterial->SetScalarParameterValue(FName("Alpha"), 0.f);
					Niagaraplayer->ServerGhostStart();
					Niagaraplayer->DynamicMaterial->SetVectorParameterValue(FName("Loc"), Niagaraplayer->GetCapsuleComponent()->GetForwardVector() * -1.f);
					Niagaraplayer->DynamicMaterial->SetScalarParameterValue(FName("Amount"), Niagaraplayer->GetCharacterMovement()->Velocity.Length() / 4);
					//info->skilltype = -1;
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
				if (ACharacter4* Niagaraplayer =Cast<ACharacter4>(OtherPlayer)) {
					Niagaraplayer->SaveCurLocation();
					FActorSpawnParameters SpawnParameters;
					SpawnParameters.Owner = OtherPlayer;
					SpawnParameters.Instigator = OtherPlayer;
					ch4skill.X = info->CH1NiaLoc.X;
					ch4skill.Y = info->CH1NiaLoc.Y;
					ch4skill.Z = info->CH1NiaLoc.Z;
					ServerTemp =GetWorld()->SpawnActor<ANiagaraActor>(NiagaraActorRef, ch4skill, FRotator::ZeroRotator, SpawnParameters);
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
					Cast<ASkill4Actor>(ServerTemp)->bTimerStart = true;
				}
			}
			if(info->p_type == PlayerType::Character4 && info->skilltype == 2){
				ACharacter4* Niagaraplayer = Cast<ACharacter4>(OtherPlayer);
				Niagaraplayer->GetNiagaraComp()->Deactivate();
				Niagaraplayer->GetMesh()->SetVisibility(true, false);
				Niagaraplayer->GetCurWeapon()->GetWeaponMesh()->SetVisibility(true);
				info->bch4end = false;
				bool Test = Niagaraplayer->GetMesh()->IsVisible();
				UE_LOG(LogTemp, Warning, TEXT("hahah : %d"), Test);
				info->skilltype = -1;
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
				DynamicMaterial = UMaterialInstanceDynamic::Create(OldMaterial, this);
				if (DynamicMaterial)
				{
					SpawnCharacter->GetMesh()->SetMaterial(0, DynamicMaterial);
					DynamicMaterial->SetScalarParameterValue(FName("Alpha"), 0.f);
				}
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
	inst->m_Socket->Send_Move_Packet(id, MyLocation, MyRotation, MyVelocity, max_speed);
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


void ACharacterController::SetHp(float recvdamaged)
{
	damaged = recvdamaged;
	//Cast<ACharacterBase>(GetOwner())->SetHealth(recvdamaged);
	//UE_LOG(LogClass, Warning, TEXT("hp : %f"), DamagedHp);
	//ACharacterBase* BaseCharacter = Cast<ACharacterBase>(GetPawn());
	//if (BaseCharacter)
	//{
	//	UE_LOG(LogClass, Warning, TEXT("hp : %f"), DamagedHp);
	//	BaseCharacter->SetHealth(DamagedHp);
	//	SetHUDHealth(BaseCharacter->GetHealth(), BaseCharacter->MaxGetHealth());
	//}
}
