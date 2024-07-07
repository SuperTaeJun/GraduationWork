#include "Character/CharacterBase.h"
//입력
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Player/CharacterController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CameraShake/Sprint.h"
#include "CameraShake/Jog.h"
#include "CameraShake/Shoot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/WeaponBase.h"
#include "Engine/SkeletalMeshSocket.h"
#include "HUD/MainHUD.h"
#include "Player/CharacterController.h"
#include "GameFramework/PlayerController.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
#include "Weapon/ProjectileBase.h"
#include "Game/BOGameMode.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "GameProp/EscapeTool.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Animatiom/BOAnimInstance.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundCue.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	TurningType = ETurningInPlace::ETIP_NotTurning;
	CharacterState = ECharacterState::ECS_DEFAULT;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));

	GetMesh()->GetAnimInstance();

	Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = 400.f;
	Movement->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = DEFAULTCAMERALENGTH;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->SetWorldLocation(FVector(-40.f, 0.f, 160.f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	Grenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade"));
	Grenade->SetupAttachment(GetMesh(), FName("GrandeSocket"));
	Grenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Grenade->bHiddenInGame = true;

	PathSorce = CreateDefaultSubobject<UArrowComponent>(TEXT("GrenadePath"));
	PathSorce->SetupAttachment(GetMesh(), FName("HeadSocket"));

	Aim = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraAim"));

	//스테이트
	MaxHealth = 100.f;
	Health = MaxHealth;
	MaxStamina = 100.f;
	Stamina = MaxStamina;
	StaminaExhaustionState = false;
	bCanFire = true;
	GrendeNum = 5;
	WallGrendeNum = 5;
	BoobyTrapNum = 5;
	//bShowSelectUi = false;
	ObtainedEscapeToolNum = 0;
	CurWeaponType = EWeaponType::ECS_DEFAULT;
	bStarted = false;
	StartedCnt = 5.f;
	bCanEscape = false;

}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();


	StartTransform = GetActorTransform();
	inst = Cast<UBOGameInstance>(GetGameInstance());
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		//입력시스템 매핑
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefalutMappingContext, 0);
		}
	}
	//MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(MainController->GetHUD()) : MainHUD;
	MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
	if (MainController)
	{
		EnableInput(MainController);
		FInputModeGameOnly GameInput;
		MainController->bShowMouseCursor = false;
		MainController->SetInputMode(GameInput);
		MainController->ShowMatchingUi();
		MainController->SetHUDMatchingUi();
	}
	////무기선택 ui생성
	//MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
	//if (MainController)
	//{
	//	SetWeaponUi();
	//}

	BojoMugiType = EBojoMugiType::ECS_DEFAULT;

	if (MainController)
	{
		MainController->SetHUDSkill();
		UpdateHpHUD();
		UpdateStaminaHUD();
		UpdateObtainedEscapeTool();
		MainController->SetHUDBojoImage(BojoMugiType);

	}
	if(Aim)
		Aim->SetAutoActivate(false);


	OnTakeAnyDamage.AddDynamic(this, &ACharacterBase::ReciveDamage);

}


void ACharacterBase::UpdateCameraBoom(float DeltaTime)
{
	if (CameraBoom->TargetArmLength <= DEFAULTCAMERALENGTH && (CharacterState == ECharacterState::ECS_RUN || CharacterState == ECharacterState::ECS_IDLE))
	{
		CameraBoom->TargetArmLength += DeltaTime * 400;
	}
	else if (CameraBoom->TargetArmLength >= SPRINTCAMERALENGTH && CharacterState == ECharacterState::ECS_SPRINT)
	{
		CameraBoom->TargetArmLength -= DeltaTime * 400;
	}
}
void ACharacterBase::UpdateStamina(float DeltaTime)
{
	if (StaminaExhaustionState == false)
	{
		if (CharacterState == ECharacterState::ECS_SPRINT && Stamina >= 0.f)
		{
			Stamina -= 0.4f;
			if (Stamina <= 0.f)
				StaminaExhaustionState = true;
		}
		else 	if ((CharacterState == ECharacterState::ECS_RUN || CharacterState == ECharacterState::ECS_IDLE) && Stamina < MaxStamina)
		{
			Stamina += 0.2f;
		}
	}
	else	if (StaminaExhaustionState == true)
	{
		if (Stamina < 50.f)
		{
			Stamina += 0.2f;
		}
		else if (Stamina >= 50.f)
		{
			StaminaExhaustionState = false;
		}
	}
	UpdateStaminaHUD();
}

void ACharacterBase::SetHUDCrosshair(float DeltaTime)
{
	MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
	//MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(MainController->GetHUD()) : MainHUD;

	if (MainController)
	{
		FCrosshairPackage HUDPackage;

		HUDPackage.CrosshairCenter = CrosshairsCenter;
		HUDPackage.CrosshairLeft = CrosshairsLeft;
		HUDPackage.CrosshairRight = CrosshairsRight;
		HUDPackage.CrosshairTop = CrosshairsTop;
		HUDPackage.CrosshairBottom = CrosshairsBottom;

		MainController->SetHUDCrosshair(HUDPackage);
	}

}

void ACharacterBase::UpdateHpHUD()
{
	MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
	if (MainController)
	{
		MainController->SetHUDHealth(FMath::Clamp(Health,0.f,100.f), MaxHealth);
	}
}

void ACharacterBase::UpdateStaminaHUD()
{
	MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
	if (MainController)
	{
		MainController->SetHUDStamina(FMath::Clamp(Stamina, 0.f, 100.f) , MaxStamina);
	}
}

void ACharacterBase::UpdateObtainedEscapeTool()
{
	MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
	if (MainController)
	{
		MainController->SetHUDEscapeTool(ObtainedEscapeToolNum);
	}
}

void ACharacterBase::SetWeaponUi()
{
	if (MainController)
	{
		FInputModeUIOnly UiGameInput;
		MainController->SetInputMode(UiGameInput);
		MainController->DisableInput(MainController);
		//bShowSelectUi = true;
		MainController->bShowMouseCursor = true;
		MainController->bEnableMouseOverEvents = true;

		MainController->showWeaponSelect();
	}

}

void ACharacterBase::SetRespawnUi()
{
	FInputModeUIOnly UiGameInput;
	if (MainController)
	{
		MainController->SetInputMode(UiGameInput);
		MainController->DisableInput(MainController);
		//bShowSelectUi = true;
		MainController->bShowMouseCursor = true;
		MainController->bEnableMouseOverEvents = true;
		MainController->ShowRespawnSelect();
	}
}

void ACharacterBase::SetResetState()
{
	Health = 100.f;
	Stamina = 100.f;
	UpdateHpHUD();
	UpdateStaminaHUD();
	CurWeapon->Destroy();
	CurWeapon = nullptr;
	bDissolve = false;
	DissolvePercent = -1.f;
	GetMesh()->SetMaterial(0, MDynamicDissolveInst);
	MDynamicDissolveInst->SetScalarParameterValue(FName("Dissolve"), DissolvePercent);

	//여기서 패킷
	if (inst)
		inst->m_Socket->Send_Remove_Weapon(inst->GetPlayerID(), true);

}

void ACharacterBase::SetWeapon(TSubclassOf<class AWeaponBase> Weapon, FName SocketName)
{
	if (!CurWeapon)
	{
		RightSocketName = SocketName;

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.Instigator = this;
		AActor* SpawnWeapon = GetWorld()->SpawnActor<AWeaponBase>(Weapon, SpawnParameters);
		CurWeapon = Cast<AWeaponBase>(SpawnWeapon);

		//UE_LOG(LogTemp, Warning, TEXT("SPAWN WEAPON"));

		const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(SocketName);


		if (WeaponSocket && CurWeapon)
		{
			WeaponSocket->AttachActor(CurWeapon, GetMesh());
			CurWeapon->SetOwner(this);
		}

		SetRun();
	}

}

void ACharacterBase::SetbCanObtainEscapeTool(bool _bCanObtain)
{
	bCanObtainEscapeTool = _bCanObtain;
}


void ACharacterBase::SetHealth(float Damaged)
{
	Health = Damaged;
}


void ACharacterBase::PlayFireActionMontage()
{
	if(FireActionMontage)
		PlayAnimMontage(FireActionMontage,1.5f);

}

void ACharacterBase::GrandeThrow()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	Cast<UBOAnimInstance>(AnimInstance)->bUseLeftHand = false;
	//PlayAnimMontage(GrenadeMontage, 2.f, FName("Fire"));

}
void ACharacterBase::GrandeAim()
{
	CurWeapon->SetActorHiddenInGame(true);
	Grenade->bHiddenInGame = false;

	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(FName("LeftHandSocket"));

	if (WeaponSocket && CurWeapon)
	{
		WeaponSocket->AttachActor(CurWeapon, GetMesh());
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && GrenadeMontage)
		PlayAnimMontage(GrenadeMontage, 1.5f);
}
void ACharacterBase::GrandeThrowFinish()
{

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	Cast<UBOAnimInstance>(AnimInstance)->bUseLeftHand = true;
	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(RightSocketName);

	CurWeapon->SetActorHiddenInGame(false);

	if (WeaponSocket && CurWeapon)
	{
		WeaponSocket->AttachActor(CurWeapon, GetMesh());
	}
	GrendeNum -= 1;

	//Grenade->bHiddenInGame = false;
	Grenade->SetHiddenInGame(true);
}

void ACharacterBase::SpawnGrenade()
{
	//Grenade->bHiddenInGame = true;
	Grenade->SetHiddenInGame(true);

	switch (BojoMugiType)
	{
	case EBojoMugiType::E_Grenade:
		if (GrendeNum > 0)
		{
			SetSpawnGrenade(GrenadeClass);
			GrendeNum -= 1;
		}
		break;
	case EBojoMugiType::E_Wall:
		if (WallGrendeNum > 0)
		{
			SetSpawnGrenade(WallClass);
			WallGrendeNum -= 1;
		}
		break;
	//case EBojoMugiType::E_BoobyTrap:
	//	SetSpawnGrenade(BoobyTrapClass);
	//	break;
	case EBojoMugiType::ECS_DEFAULT:
		SetSpawnGrenade(GrenadeClass);
		break;
	}

}

void ACharacterBase::ReloadForMontage()
{
	CurWeapon->CurAmmo = CurWeapon->MaxAmmo;
	MainController->SetHUDAmmo(CurWeapon->CurAmmo);
	bCanFire = true;
}

void ACharacterBase::SetSpawnGrenade(TSubclassOf<AProjectileBase> Projectile)
{
	//UE_LOG(LogTemp, Log, TEXT("GRENDADE SPAWN"));
	if (Grenade)
	{
		const FVector StartLocation = GetMesh()->GetSocketLocation(FName("GrandeSocket"));
		FVector ToHitTarget = HitTarget - StartLocation;
		FActorSpawnParameters SpawnParms;
		SpawnParms.Owner = this;
		SpawnParms.Instigator = this;
		TObjectPtr<UWorld> World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectileBase>(Projectile, StartLocation, ToHitTarget.Rotation(), SpawnParms);
			if (Cast<UBOGameInstance>(GetGameInstance()))
			{
				switch (BojoMugiType)
				{
				case EBojoMugiType::E_Grenade:
					Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Fire_Effect(_SessionId, StartLocation, ToHitTarget.Rotation(), 2);
					break;
				case EBojoMugiType::E_Wall:
					Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Fire_Effect(_SessionId, StartLocation, ToHitTarget.Rotation(), 3);
					break;

				}
					
			}
		}
	}
}

void ACharacterBase::ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser)
{
	if (MainController)
		MainController->SeverHpSync(Health, inst->GetPlayerID());
	
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHpHUD();
	ACharacterBase* DamageInsigatorCh= Cast<ACharacterBase>(InstigatorController->GetPawn());

	
	if (Health <= 0.0f)
	{
		bDissolve = true;
		if (DamageInsigatorCh)
		{
			//서버
			if (ObtainedEscapeToolNum > 0 && 10 > ObtainedEscapeToolNum)
			{
				DamageInsigatorCh->SetEscapeToolNum(DamageInsigatorCh->ObtainedEscapeToolNum + 1);
				ObtainedEscapeToolNum -= 1;
				if (inst) {
					//ui에서 상대방의 아이템 개수 늘려주고
					Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Increase_item_count_packet(DamageInsigatorCh->_SessionId, DamageInsigatorCh->GetEscapeToolNum());
					// 내꺼는 줄여줌 
					Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Decrease_item_count_packet(inst->GetPlayerID(), ObtainedEscapeToolNum);
					
				}
			
			}
			else if (10 <= ObtainedEscapeToolNum)
			{
				DamageInsigatorCh->SetEscapeToolNum(DamageInsigatorCh->ObtainedEscapeToolNum + 4);
				ObtainedEscapeToolNum -= 4;
				if (inst) {
					Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Increase_item_count_packet(DamageInsigatorCh->_SessionId, DamageInsigatorCh->GetEscapeToolNum());
					Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Decrease_item_count_packet(inst->GetPlayerID(), ObtainedEscapeToolNum);
				}
			}
		}
		if(CurWeapon)
			CurWeapon->CurAmmo = 0;
		GetWorld()->GetTimerManager().SetTimer(DeadTimer, this, &ACharacterBase::Dead, DeadTime, false);
		if(MainController)
			DisableInput(MainController);
		UpdateObtainedEscapeTool();

		if (bAlive)
		{
			bAlive = false;
			PlayAnimMontage(DeadMontage);
			//MainController->ServerDeadSync(bAlive, inst->GetPlayerID());
		}
	}
}

void ACharacterBase::Dead()
{
	bCanFire = true;
	SetRespawnUi();
	//ABOGameMode* GameMode = GetWorld()->GetAuthGameMode<ABOGameMode>();
	//if (GameMode)
	//{
	//if (CurWeapon)
	//{
	//	CurWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//	CurWeapon->Destroy();
	//}
	//	GameMode->Respawn(this, MainController);
	//}
}



void ACharacterBase::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 30.f)
	{
		TurningType = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -30.f)
	{
		TurningType = ETurningInPlace::ETIP_Left;
	}
	if (TurningType != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningType = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ACharacterBase::AimOffset(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool IsFalling = GetCharacterMovement()->IsFalling();
	//UE_LOG(LogTemp, Log, TEXT("%f"), AO_Yaw);


	if (!IsFalling && Speed == 0.f) //점프 아니고 서있을때
	{
		//UE_LOG(LogTemp, Log, TEXT("%f"), Speed);
		FRotator CurAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurAimRotation, StartingAimRotation);
		AO_Yaw = DeltaRotation.Yaw;
		if (TurningType == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (IsFalling || Speed > 0.f) // 뛰거나 점프일때
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningType = ETurningInPlace::ETIP_NotTurning;
	}
	AO_Pitch = GetBaseAimRotation().Pitch + 20.f;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ACharacterBase::StartFireTimer()
{
	GetWorldTimerManager().SetTimer(FireTimer, this, &ACharacterBase::FireTimerFinished, CurWeapon->GetFirerate());
}

void ACharacterBase::FireTimerFinished()
{
	bCanFire = true;
	if (bFirePressed)
		Fire();
}

void ACharacterBase::Fire()
{
	if (CurWeapon->CurAmmo <= 0)
		bCanFire = false;

	//UE_LOG(LogTemp, Log, TEXT("FIRE"));
	if (bCanFire == true)
	{
		bCanFire = false;
		if (CurWeapon) 
		{
			//UE_LOG(LogTemp, Warning, TEXT("FIRE"));
			CurWeapon->Fire(HitTarget);
		}
		PlayFireActionMontage();

		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraShake(UShoot::StaticClass());

		CurWeapon->CurAmmo -= 1;
		MainController->SetHUDAmmo(CurWeapon->CurAmmo );

		StartFireTimer();
	}
}

void ACharacterBase::FirePressd(bool _Pressd)
{
	if (_Pressd && CharacterState!= ECharacterState::ECS_SPRINT && CurWeapon)
	{
		Fire();
	}
}

void ACharacterBase::TraceUnderCrossHiar(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	//크로스헤어 뷰포트에서의 좌표
	FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	FVector CrossHairPostion;
	FVector CrossHairDirection;

	//주어진 2D 화면 공간 좌표를 3D 세계 공간 지점과 방향으로 변환
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld
	(
		UGameplayStatics::GetPlayerController(this, 0),
		CrossHairLocation,
		CrossHairPostion,
		CrossHairDirection
	);
	if (bScreenToWorld)
	{
		FVector Start = CrossHairPostion;
		if (this)
		{
			float DistanceToCharacter = (this->GetActorLocation() - Start).Size();
			Start += CrossHairDirection * (DistanceToCharacter + 100.f);
		}

		if (CurWeapon)
		{
			FVector End = Start + CrossHairDirection * CurWeapon->Range;

			GetWorld()->LineTraceSingleByChannel(
				TraceHitResult,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			if (!TraceHitResult.bBlockingHit)
			{
				TraceHitResult.ImpactPoint = End;
			}
			else
			{
				//DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
			}
		}
	}
	
}
void ACharacterBase::SetSprint()
{
	Movement->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}
void ACharacterBase::SetRun()
{
	Movement->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
}
enum Move {
	Move_Player
};
void ACharacterBase::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	/*if (MovementVector.Length() != 0)
	{
		
		ACharacterController* PlayerController = Cast<ACharacterController>(GetController());
		PlayerController->UpdatePlayer(Move_Player);
	}*/
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotaion(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDir = FRotationMatrix(YawRotaion).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDir, MovementVector.Y);
	const FVector RightDir = FRotationMatrix(YawRotaion).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDir, MovementVector.X);
}

void ACharacterBase::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	if (GetController())
	{
		AddControllerYawInput(LookAxis.X);
		AddControllerPitchInput(LookAxis.Y);
	}
}

void ACharacterBase::Sprint_S(const FInputActionValue& Value)
{
	if (!StaminaExhaustionState && CurWeapon)
	{
		CharacterState = ECharacterState::ECS_SPRINT;
		//Movement->bOrientRotationToMovement = true;
		//bUseControllerRotationYaw = false;

		SetSprint();
	}
	else
	{
		CharacterState = ECharacterState::ECS_RUN;
		//Movement->bOrientRotationToMovement = false;
		//bUseControllerRotationYaw = true;
		SetRun();
	}
}
void ACharacterBase::Sprint_E(const FInputActionValue& Value)
{
	CharacterState = ECharacterState::ECS_RUN;
	Movement->MaxWalkSpeed = 400;
	//Movement->bOrientRotationToMovement = false;
	//bUseControllerRotationYaw = true;
	SetRun();
}
void ACharacterBase::Fire_S(const FInputActionValue& Value)
{
	if (CurWeapon)
	{
		if (CurWeapon->CurAmmo <= 0)
		{
			bFirePressed = false;
		}
		else if (CurWeapon->CurAmmo >= 1)
		{
			bFirePressed = true;
		}
		FirePressd(bFirePressed);

	}

}
void ACharacterBase::Fire_E(const FInputActionValue& Value)
{
	bFirePressed = false;
	FirePressd(bFirePressed);
}
void ACharacterBase::Inter(const FInputActionValue& Value)
{
	if (bCanObtainEscapeTool && OverlappingEscapeTool)
	{
		ObtainedEscapeToolNum += 1;
		//패킷(id, num)
		if (inst)
			Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Item_packet(inst->GetPlayerID(), ObtainedEscapeToolNum);
		UpdateObtainedEscapeTool();
		OverlappingEscapeTool->SetHideMesh();
		OverlappingEscapeTool = nullptr;
	}
	else if (!bCanObtainEscapeTool && OverlappingEscapeTool)
	{
		//UE_LOG(LogTemp, Log, TEXT("TEST"));
		EToolTranfrom(Value);
	}

	if (bCanEscape)
	{
		if (inst)
			Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_End_Game_packet(inst->GetPlayerID());
		//GetWorld()->ServerTravel(FString("/Game/Maps/GameRoom"), false,true);
	}
}
void ACharacterBase::Inter_Start(const FInputActionValue& Value)
{
		if (!bCanObtainEscapeTool && OverlappingEscapeTool)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			Cast<UBOAnimInstance>(AnimInstance)->bUseLeftHand = false;
			if (InterMontage) {
				PlayAnimMontage(InterMontage);
				if (inst)
					inst->m_Socket->Send_item_Anim_packet(inst->GetPlayerID(), 0);
			}
		}
}
void ACharacterBase::Inter_End(const FInputActionValue& Value)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	Cast<UBOAnimInstance>(AnimInstance)->bUseLeftHand = true;
	if (InterMontage) {
		StopAnimMontage(InterMontage);
		if (inst)
			inst->m_Socket->Send_item_Anim_packet(inst->GetPlayerID(), 1);
	}

}
void ACharacterBase::EToolTranfrom(const FInputActionValue& Value)
{
	//if (OverlappingEscapeTool)
	//{
	OverlappingEscapeTool->TransformMesh(GetWorld()->GetDeltaSeconds(),false,false);
	//}

}
void ACharacterBase::Reroad(const FInputActionValue& Value)
{
	if (CurWeapon)
	{
		if (ReloadMontage)
		{
			PlayAnimMontage(ReloadMontage);
			if (inst)
				inst->m_Socket->Send_Reload_packet(inst->GetPlayerID(), true);
			bCanFire = false;
		}
		if (CurWeapon->GetReloadSound())
		{
			UGameplayStatics::PlaySoundAtLocation(this, CurWeapon->GetReloadSound(), GetActorLocation());
		}
	}
}

void ACharacterBase::GrandeFire_Aiming(const FInputActionValue& Value)
{
	if (CurWeapon)
	{
		Aim->Activate();
		FPredictProjectilePathParams Path;
		Path.StartLocation = PathSorce->GetComponentLocation();
		Path.LaunchVelocity = FollowCamera->GetForwardVector() * 1500.f;
		Path.ProjectileRadius = 3.f;
		Path.bTraceWithCollision = true;
		Path.ActorsToIgnore.Add(this);
		//Path.DrawDebugType = EDrawDebugTrace::ForOneFrame;
		FPredictProjectilePathResult Result;
		UGameplayStatics::PredictProjectilePath(Grenade, Path, Result);
		TArray<FVector> Locations;
		for (auto OnePathData : Result.PathData)
		{
			Locations.Add(OnePathData.Location);
		}
		SWAimLastLoc = Locations.Last();
		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(Aim, FName("PositionArray"), Locations);
	}
}

void ACharacterBase::GrandeFire(const FInputActionValue& Value)
{
	if (CurWeapon)
	{
		Aim->Deactivate();

		if (BojoMugiType == EBojoMugiType::E_BoobyTrap)
		{
			TObjectPtr<UWorld> World = GetWorld();
			if (World && BoobyTrapNum>0)
			{
				FActorSpawnParameters SpawnParms;
				SpawnParms.Owner = this;

				World->SpawnActor<AProjectileBase>(BoobyTrapClass, SWAimLastLoc, FRotator::ZeroRotator, SpawnParms);
				BoobyTrapNum -= 1;
				//여기 부비트랩
				if (Cast<UBOGameInstance>(GetGameInstance()))
				{
					Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Fire_Effect(_SessionId, SWAimLastLoc, FRotator::ZeroRotator, 4);
				}
			}
		}
		else
			GrandeAim();

	}
}

void ACharacterBase::SelectGrande(const FInputActionValue& Value)
{
	BojoMugiType = EBojoMugiType::E_Grenade;

	MainController->SetHUDBojoImage(BojoMugiType);
}

void ACharacterBase::SelectWall(const FInputActionValue& Value)
{
	BojoMugiType = EBojoMugiType::E_Wall;
	MainController->SetHUDBojoImage(BojoMugiType);
}

void ACharacterBase::SelectTrap(const FInputActionValue& Value)
{
	BojoMugiType = EBojoMugiType::E_BoobyTrap;
	MainController->SetHUDBojoImage(BojoMugiType);
}

void ACharacterBase::StartJump(const FInputActionValue& Value)
{
	if (CanJump && CurWeapon)
		Super::Jump();

	CanJump = false;

}

void ACharacterBase::StopJump(const FInputActionValue& Value)
{
	Super::StopJumping();
}

void ACharacterBase::Skill_S(const FInputActionValue& Value)
{

}

void ACharacterBase::Skill_E(const FInputActionValue& Value)
{
}

void ACharacterBase::Detect_E(const FInputActionValue& Value)
{
	if (CurWeapon)
	{
		CurWeapon->SetDetectNiagara(false);
	}
}

void ACharacterBase::Detect_S(const FInputActionValue& Value)
{
	if (CurWeapon)
	{
		CurWeapon->DetectTool(HitTarget);
		CurWeapon->SetDetectNiagara(true);
	}
}

void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetVelocity().Size() <= 0.f)
		CharacterState = ECharacterState::ECS_IDLE;
	switch (CharacterState)
	{
	case ECharacterState::ECS_IDLE:
		UpdateCameraBoom(DeltaTime);
		break;
	case ECharacterState::ECS_RUN:
		Movement->MaxWalkSpeed = 400.f;
		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraShake(UJog::StaticClass());
		UpdateCameraBoom(DeltaTime);
		break;
	case ECharacterState::ECS_SPRINT:
		if (!StaminaExhaustionState)
		{
			if (!bSkillUsing)
			{
				Movement->MaxWalkSpeed = 600.f;
				UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraShake(USprint::StaticClass());
			}
		}
		UpdateCameraBoom(DeltaTime);
		break;
	case ECharacterState::ECS_DEFAULT:
		Movement->MaxWalkSpeed = 400.f;
		UpdateCameraBoom(DeltaTime);
		break;
	}

	if (!Movement->IsFalling())
	{
		CanJump = true;
	}

	UpdateStamina(DeltaTime);
	AimOffset(DeltaTime);

	SetHUDCrosshair(DeltaTime);
	FHitResult HitResult;
	TraceUnderCrossHiar(HitResult);
	HitTarget = HitResult.ImpactPoint;

	if (!PathSorce->bHiddenInGame)
	{
		PathSorce->SetHiddenInGame(true);
	}

	//캐릭터 디졸브
	if (bDissolve)
	{
		if (MDissolveInst)
		{
			MDynamicDissolveInst = UMaterialInstanceDynamic::Create(MDissolveInst, this);
			if (MDynamicDissolveInst)
			{
				DissolvePercent += DeltaTime/4;
				GetMesh()->SetMaterial(0, MDynamicDissolveInst);
				MDynamicDissolveInst->SetScalarParameterValue(FName("Dissolve"), DissolvePercent);
			}
		}
	}


	if (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->m_Socket->bAllReady == true && !bStarted)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartGame"));
		Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->m_Socket->bAllReady = false;
		bStarted = true;
		//DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		GetWorldTimerManager().SetTimer(StartHandle, this, &ACharacterBase::StartGame, 5.f);
	}


}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharacterBase::Move);// trigger 매 틱마다
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACharacterBase::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacterBase::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacterBase::StopJump);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ACharacterBase::Sprint_S);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACharacterBase::Sprint_E);
		EnhancedInputComponent->BindAction(InterAction, ETriggerEvent::Triggered, this, &ACharacterBase::Inter);
		EnhancedInputComponent->BindAction(InterAction, ETriggerEvent::Started, this, &ACharacterBase::Inter_Start);
		EnhancedInputComponent->BindAction(InterAction, ETriggerEvent::Completed, this, &ACharacterBase::Inter_End);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ACharacterBase::Fire_S);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ACharacterBase::Fire_E);
		EnhancedInputComponent->BindAction(ReRoadAction, ETriggerEvent::Triggered, this, &ACharacterBase::Reroad);
		EnhancedInputComponent->BindAction(GrandeFireAction, ETriggerEvent::Triggered, this, &ACharacterBase::GrandeFire_Aiming);
		EnhancedInputComponent->BindAction(GrandeFireAction, ETriggerEvent::Completed, this, &ACharacterBase::GrandeFire);
		EnhancedInputComponent->BindAction(SelectGrandeAction, ETriggerEvent::Triggered, this, &ACharacterBase::SelectGrande);
		EnhancedInputComponent->BindAction(SelectWallAction, ETriggerEvent::Triggered, this, &ACharacterBase::SelectWall);
		EnhancedInputComponent->BindAction(SelectTrapAction, ETriggerEvent::Triggered, this, &ACharacterBase::SelectTrap);
		EnhancedInputComponent->BindAction(DetectAction, ETriggerEvent::Triggered, this, &ACharacterBase::Detect_S);
		EnhancedInputComponent->BindAction(DetectAction, ETriggerEvent::Completed, this, &ACharacterBase::Detect_E);
	}
}

void ACharacterBase::SpawnBeam(FVector StartBeam, FVector EndBeam)
{

	UE_LOG(LogClass, Warning, TEXT("SB %f, EB : %f"), StartBeam.X, EndBeam.X);
	UNiagaraComponent* Beam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurWeapon->BeamNiagara, StartBeam);
	if (Beam)
	{
		Beam->SetVectorParameter(FName("End"), EndBeam);
	}
}

void ACharacterBase::SpawnHitImpact(FVector HitLoc, FRotator HitRot)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation
	(
		GetWorld(),
		CurWeapon->ImpactNiagara,
		HitLoc,
		HitRot
	);
}

void ACharacterBase::StartGame()
{

	Movement->Velocity = FVector::ZeroVector;
	MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
	if (MainController)
	{
		//if (MainHUD)
		//{
		//	UE_LOG(LogTemp, Warning, TEXT("ADDTOOLNUM"));
		//	MainHUD->AddToolNumUi();
		//}
		/*if (inst)
			inst->m_Socket->Send_Start_game_packet(inst->GetPlayerID());*/
		MainController->MainHUD->AddToolNumUi();
		// num 계수, name 처리 
		MainController->SetNum();
		MainController->SetChName();
		SetActorTransform(StartTransform);
		MainController->MainHUD->RemoveMatchingUi();
		//bStarted = false;
		EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		SetWeaponUi();
	
	}

}

void ACharacterBase::Server_PlayDeadAnim()
{
	PlayAnimMontage(DeadMontage);
}
