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
#include "Weapon/ProjectileBase.h"
#include "Game/BOGameMode.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "GameProp/EscapeTool.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"



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
	Movement->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

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
	GrendeNum = 10;
	WallGrendeNum = 10;
	BoobyTrapNum = 10;
	//bShowSelectUi = false;
	ObtainedEscapeToolNum = 0;
	CurWeaponType = EWeaponType::ECS_DEFAULT;
	bStarted = false;
}

//float ACharacterBase::GetAO_Yaw()
//{
//	AO_Yaw = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation()).Yaw;
//
//	return AO_Yaw;
//}
//
//float ACharacterBase::GetAO_Pitch()
//{
//	AO_Pitch = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation()).Pitch;
//	return AO_Pitch;
//}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		//입력시스템 매핑
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefalutMappingContext, 0);
		}
	}

	if (MainHUD)
	{
		MainHUD->AddMatchingUi();
	}
	////무기선택 ui생성
	//MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
	//if (MainController)
	//{
	//	SetWeaponUi();
	//}

	BojoMugiType = EBojoMugiType::ECS_DEFAULT;

	OnTakeAnyDamage.AddDynamic(this, &ACharacterBase::ReciveDamage);

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

}


void ACharacterBase::UpdateSprintCamera(float DeltaTime)
{
	if(CameraBoom->TargetArmLength>= SPRINTCAMERALENGTH && CharacterState == ECharacterState::ECS_SPRINT)
		CameraBoom->TargetArmLength -= DeltaTime * 400;
	else if (CameraBoom->TargetArmLength <= DEFAULTCAMERALENGTH && CharacterState == ECharacterState::ECS_RUN)
		CameraBoom->TargetArmLength += DeltaTime * 400;
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
	FInputModeUIOnly UiGameInput;
	if (MainController)
	{
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

void ACharacterBase::SetWeapon(TSubclassOf<class AWeaponBase> Weapon, FName SocketName)
{
	if (!CurWeapon)
	{
		RightSocketName = SocketName;
		AActor* SpawnWeapon = GetWorld()->SpawnActor<AWeaponBase>(Weapon);
		CurWeapon = Cast<AWeaponBase>(SpawnWeapon);

		//UE_LOG(LogTemp, Warning, TEXT("SPAWN WEAPON"));

		const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(SocketName);


		if (WeaponSocket && SpawnWeapon)
		{
			WeaponSocket->AttachActor(SpawnWeapon, GetMesh());
			SpawnWeapon->SetOwner(this);
		}

	}

}

void ACharacterBase::SetbCanObtainEscapeTool(bool _bCanObtain)
{
	bCanObtainEscapeTool = _bCanObtain;
}

void ACharacterBase::SetHealth(float Damaged)
{
	Health -= Damaged;
}


void ACharacterBase::PlayFireActionMontage()
{
	if(FireActionMontage)
		PlayAnimMontage(FireActionMontage,1.5f);

}

void ACharacterBase::GrandeThrow()
{
	PlayAnimMontage(GrenadeMontage, 1.f, FName("Fire"));
	CurWeapon->SetActorHiddenInGame(true);
	//CurWeapon->SetActorHiddenInGame(true);
	//UE_LOG(LogTemp, Log, TEXT("FIRE"));
}
void ACharacterBase::GrandeAim()
{
	Grenade->bHiddenInGame = false;

	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(FName("LeftHandSocket"));

	if (WeaponSocket && CurWeapon)
	{
		WeaponSocket->AttachActor(CurWeapon, GetMesh());
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && GrenadeMontage)
		PlayAnimMontage(GrenadeMontage, 1.f, FName("Aim"));
}
void ACharacterBase::GrandeThrowFinish()
{
	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(RightSocketName);

	//CurWeapon->SetActorHiddenInGame(false);

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
		SetSpawnGrenade(GrenadeClass);
		break;
	case EBojoMugiType::E_Wall:
		SetSpawnGrenade(WallClass);
		break;
	case EBojoMugiType::E_BoobyTrap:
		SetSpawnGrenade(BoobyTrapClass);
		break;
	case EBojoMugiType::ECS_DEFAULT:
		SetSpawnGrenade(GrenadeClass);
		break;
	}

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
		//SpawnParms.Instigator = this;
		TObjectPtr<UWorld> World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectileBase>(Projectile, StartLocation, ToHitTarget.Rotation(), SpawnParms);
		}
	}
}

void ACharacterBase::ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHpHUD();


	UE_LOG(LogTemp, Warning, TEXT("RECIVE DAMAGE"));
	if (Health <= 0.0f)
	{
		ABOGameMode* GameMode = GetWorld()->GetAuthGameMode<ABOGameMode>();
		if (GameMode)
		{
			//MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
			//ACharacterController* AttackerController = Cast<ACharacterController>(InstigatorController);
			GetWorld()->GetTimerManager().SetTimer(DeadTimer, this, &ACharacterBase::Dead, DeadTime, false);
		}
	}
}

void ACharacterBase::Dead()
{
	SetRespawnUi();
	//ABOGameMode* GameMode = GetWorld()->GetAuthGameMode<ABOGameMode>();
	//if (GameMode)
	//{
	//	if (CurWeapon)
	//	{
	//		CurWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	//		CurWeapon->Destroy();
	//	}
	//	GameMode->Respawn(this, MainController);
	//}
}



void ACharacterBase::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningType = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
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
	AO_Pitch = GetBaseAimRotation().Pitch;
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
	if (!StaminaExhaustionState)
	{
		CharacterState = ECharacterState::ECS_SPRINT;
		Movement->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
	}
	else
	{
		CharacterState = ECharacterState::ECS_RUN;
		Movement->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
	}
}
void ACharacterBase::Sprint_E(const FInputActionValue& Value)
{
	CharacterState = ECharacterState::ECS_RUN;
	Movement->MaxWalkSpeed = 400;
	Movement->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
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
		UpdateObtainedEscapeTool();
		OverlappingEscapeTool->SetHideMesh();
		OverlappingEscapeTool = nullptr;
	}
	else if (!bCanObtainEscapeTool && OverlappingEscapeTool)
	{
		//UE_LOG(LogTemp, Log, TEXT("TEST"));
		EToolTranfrom(Value);
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
		CurWeapon->CurAmmo = CurWeapon->MaxAmmo;
		MainController->SetHUDAmmo(CurWeapon->CurAmmo);
	}
}

void ACharacterBase::GrandeFire_Aiming(const FInputActionValue& Value)
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
	UGameplayStatics::PredictProjectilePath(Grenade, Path,Result);
	TArray<FVector> Locations;
	for (auto OnePathData : Result.PathData)
	{
		Locations.Add(OnePathData.Location);
	}
	SWAimLastLoc =Locations.Last();
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(Aim, FName("PositionArray"), Locations);
}

void ACharacterBase::GrandeFire(const FInputActionValue& Value)
{
	Aim->Deactivate();
	if (GrendeNum > 0)
	{
		if (BojoMugiType == EBojoMugiType::E_BoobyTrap)
		{
			TObjectPtr<UWorld> World = GetWorld();
			if (World)
			{
				FActorSpawnParameters SpawnParms;
				SpawnParms.Owner = this;

				World->SpawnActor<AProjectileBase>(BoobyTrapClass, SWAimLastLoc, FRotator::ZeroRotator,SpawnParms);
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
	if (CanJump)
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
		break;
	case ECharacterState::ECS_RUN:
		Movement->MaxWalkSpeed = 400.f;
		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraShake(UJog::StaticClass());
		break;
	case ECharacterState::ECS_SPRINT:
		if (!StaminaExhaustionState)
		{
			Movement->MaxWalkSpeed = 600.f;
			UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraShake(USprint::StaticClass());
		}
		break;
	case ECharacterState::ECS_FALLING:
		break;
	case ECharacterState::ECS_DEFAULT:
		Movement->MaxWalkSpeed = 400.f;
		break;
	}

	if (!Movement->IsFalling())
	{
		CanJump = true;
	}

	//UE_LOG(LogTemp, Warning, TEXT("HHHHHH : %s"),*GetVelocity().ToString());

	UpdateStamina(DeltaTime);
	UpdateSprintCamera(DeltaTime);
	AimOffset(DeltaTime);

	SetHUDCrosshair(DeltaTime);
	FHitResult HitResult;
	TraceUnderCrossHiar(HitResult);
	HitTarget = HitResult.ImpactPoint;

	if (!PathSorce->bHiddenInGame)
	{
		PathSorce->SetHiddenInGame(true);
	}


	if (/*Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->m_Socket->bAllReady == true &&*/ !bStarted)
	{
		Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->m_Socket->bAllReady = false;
		bStarted = true;
		DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		GetWorldTimerManager().SetTimer(StartHandle, this, &ACharacterBase::StartGame, 7.f);
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

	EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
	if (MainController)
	{
		SetWeaponUi();
	}
}
