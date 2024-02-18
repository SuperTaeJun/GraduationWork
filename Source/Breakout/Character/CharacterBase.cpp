#include "Character/CharacterBase.h"
//입력
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

//스킬
#include "Skill/SkillComponent.h"

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
#include "GameProp/EscapeTool.h"

ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	TurningType = ETurningInPlace::ETIP_NotTurning;
	CharacterState = ECharacterState::ECS_DEFAULT;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));

	Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = 400.f;
	Movement->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;

	SkillComp = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComp"));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = DEFAULTCAMERALENGTH;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	Grenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade"));
	Grenade->SetupAttachment(GetMesh(), FName("GrandeSocket"));
	Grenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Grenade->bHiddenInGame = true;

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
}

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

	//무기선택 ui생성
	MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
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

	if (MainController && GetWorld()->GetGameInstance())
	{
		//캐릭터 선택 (게임룸에서 선택한 정보를 게임인스턴스에서 가져와서 선택)
		switch (Cast<UBOGameInstance>(GetWorld()->GetGameInstance())->GetCharacterType())
		{
		case ECharacterType::ECharacter1:
			GetMesh()->SetSkeletalMeshAsset(Character1);
			SkillComp->SetSelectedSkill(ESelectedSkill::E_Skill1);
			break;
		case ECharacterType::ECharacter2:
			GetMesh()->SetSkeletalMeshAsset(Character2);
			SkillComp->SetSelectedSkill(ESelectedSkill::E_Skill2);
			break;
		case ECharacterType::ECharacter3:
			GetMesh()->SetSkeletalMeshAsset(Character3);
			SkillComp->SetSelectedSkill(ESelectedSkill::E_Skill3);
			break;
		case ECharacterType::ECharacter4:
			GetMesh()->SetSkeletalMeshAsset(Character4);
			SkillComp->SetSelectedSkill(ESelectedSkill::E_Skill4);
			break;

		default:
			GetMesh()->SetSkeletalMeshAsset(Character1);
			SkillComp->SetSelectedSkill(ESelectedSkill::E_Skill1);
			break;
		}
	}

	BojoMugiType = EBojoMugiType::ECS_DEFAULT;

	OnTakeAnyDamage.AddDynamic(this, &ACharacterBase::ReciveDamage);

	UpdateHpHUD();
	UpdateStaminaHUD();
	UpdateObtainedEscapeTool();
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

void ACharacterBase::SetWeapon(TSubclassOf<class AWeaponBase> Weapon, FName SocketName)
{
	//if (!CurWeapon)
	//{
	RightSocketName = SocketName;

	AActor* SpawnWeapon = GetWorld()->SpawnActor<AWeaponBase>(Weapon);
	CurWeapon = Cast<AWeaponBase>(SpawnWeapon);

	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(SocketName);


	if (WeaponSocket && SpawnWeapon)
	{
		WeaponSocket->AttachActor(SpawnWeapon, GetMesh());
	}
	SpawnWeapon->SetOwner(this);
	//}
	//else
	//{
	//	CurWeapon = nullptr;
	//	AActor* SpawnWeapon = GetWorld()->SpawnActor<AWeaponBase>(Weapon);
	//	CurWeapon = Cast<AWeaponBase>(SpawnWeapon);

	//	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(FName("WeaponSocket"));

	//	if (WeaponSocket && SpawnWeapon)
	//	{
	//		//UE_LOG(LogTemp, Log, TEXT("WEAONEQP"));
	//		WeaponSocket->AttachActor(SpawnWeapon, GetMesh());
	//	}

	//}


}

void ACharacterBase::SetbCanObtainEscapeTool(bool _bCanObtain)
{
	bCanObtainEscapeTool = _bCanObtain;
}



void ACharacterBase::PlayFireActionMontage(bool bAiming)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireActionMontage)
	{
		AnimInstance->Montage_Play(FireActionMontage);
		FName SectionName = FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ACharacterBase::GrandeThrow()
{
	Grenade->bHiddenInGame = false;

	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(FName("LeftHandSocket"));

	if (WeaponSocket && CurWeapon)
	{
		WeaponSocket->AttachActor(CurWeapon, GetMesh());
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && GrenadeMontage)
		AnimInstance->Montage_Play(GrenadeMontage);

}
void ACharacterBase::GrandeThrowFinish()
{
	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(RightSocketName);

	if (WeaponSocket && CurWeapon)
	{
		WeaponSocket->AttachActor(CurWeapon, GetMesh());
	}
	GrendeNum -= 1;
}

void ACharacterBase::ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHpHUD();


	if (Health <= 0.0f)
	{
		ABOGameMode* GameMode = GetWorld()->GetAuthGameMode<ABOGameMode>();
		if (GameMode)
		{
			MainController = MainController == nullptr ? Cast<ACharacterController>(Controller) : MainController;
			ACharacterController* AttackerController = Cast<ACharacterController>(InstigatorController);
			GameMode->PlayerRemove(this, MainController, AttackerController);
		}
	}
}

void ACharacterBase::Dead()
{

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (MainController)
		DisableInput(MainController);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}
void ACharacterBase::SpawnGrenade()
{
	Grenade->bHiddenInGame = true;

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
	UE_LOG(LogTemp, Log, TEXT("GRENDADE SPAWN"));
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
		}
	}
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
		if (CurWeapon) {
			CurWeapon->Fire(HitTarget);
		}
		PlayFireActionMontage(false);

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
	FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrossHairPostion;
	FVector CrossHairDirection;
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

		FVector End = Start + CrossHairDirection * 10000.f;

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

void ACharacterBase::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

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

void ACharacterBase::GrandeFire(const FInputActionValue& Value)
{
	if (GrendeNum > 0)
	{
		GrandeThrow();
	}
}

void ACharacterBase::SelectGrande(const FInputActionValue& Value)
{
	BojoMugiType = EBojoMugiType::E_Grenade;

	//할것
	//UI연결해야함
}

void ACharacterBase::SelectWall(const FInputActionValue& Value)
{
	BojoMugiType = EBojoMugiType::E_Wall;
	//할것
	//UI연결해야함
}

void ACharacterBase::SelectTrap(const FInputActionValue& Value)
{
	BojoMugiType = EBojoMugiType::E_BoobyTrap;
	//할것
	//UI연결해야함
}

void ACharacterBase::Skill_S(const FInputActionValue& Value)
{
	switch (SkillComp->GetSelectedSkill())
	{
	case ESelectedSkill::E_Skill1:
		SkillComp->SetIsReverse(true);
		break;
	case ESelectedSkill::E_Skill2:
		if (SkillComp->GetDashPoint() > 0 && !Movement->IsFalling())  
		{
			SkillComp->SetIsDash(true);
			SkillComp->DashStart();
		}
		break;
	case ESelectedSkill::E_Skill3:
		SkillComp->SetIsGhost(true);
		break;
	case ESelectedSkill::E_Skill4:
		break;
	}
}

void ACharacterBase::Skill_E(const FInputActionValue& Value)
{
	switch (SkillComp->GetSelectedSkill())
	{
	case ESelectedSkill::E_Skill1:
		SkillComp->SetIsReverse(false);
		break;
	case ESelectedSkill::E_Skill2:
		break;
	case ESelectedSkill::E_Skill3:
		SkillComp->SetIsGhost(false);
		SkillComp->SetIsCharageTime(false);
		break;
	case ESelectedSkill::E_Skill4:
		if (SkillComp->Toggle % 2 == 1)
		{
			SkillComp->SaveCurLocation();
		}
		else
		{
			SkillComp->SetLocation();
		}
		break;
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
	UpdateStamina(DeltaTime);
	UpdateSprintCamera(DeltaTime);
	AimOffset(DeltaTime);

	SetHUDCrosshair(DeltaTime);
	FHitResult HitResult;

	TraceUnderCrossHiar(HitResult);
	HitTarget = HitResult.ImpactPoint;
}

void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharacterBase::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACharacterBase::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ACharacterBase::Sprint_S);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACharacterBase::Sprint_E);
		EnhancedInputComponent->BindAction(InterAction, ETriggerEvent::Triggered, this, &ACharacterBase::Inter);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ACharacterBase::Fire_S);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ACharacterBase::Fire_E);
		EnhancedInputComponent->BindAction(ReRoadAction, ETriggerEvent::Triggered, this, &ACharacterBase::Reroad);
		EnhancedInputComponent->BindAction(GrandeFireAction, ETriggerEvent::Triggered, this, &ACharacterBase::GrandeFire);
		EnhancedInputComponent->BindAction(SelectGrandeAction, ETriggerEvent::Triggered, this, &ACharacterBase::SelectGrande);
		EnhancedInputComponent->BindAction(SelectWallAction, ETriggerEvent::Triggered, this, &ACharacterBase::SelectWall);
		EnhancedInputComponent->BindAction(SelectTrapAction, ETriggerEvent::Triggered, this, &ACharacterBase::SelectTrap);
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &ACharacterBase::Skill_S);
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Completed, this, &ACharacterBase::Skill_E);
	}
}

