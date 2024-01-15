#include "Character/CharacterBase.h"
//�Է�
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"

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

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = DEFAULTCAMERALENGTH;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag;
	CameraBoom->bEnableCameraRotationLag;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	//������Ʈ
	MaxHealth = 100.f;
	Health = MaxHealth;
	MaxStamina = 100.f;
	Stamina = MaxStamina;
	StaminaExhaustionState = false;
	bCanFire = true;

	//bShowSelectUi = false;
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefalutMappingContext, 0);
		}
	}

	//���⼱�� ui����
	MainController = Cast<ACharacterController>(Controller);
	MainHUD = Cast<AMainHUD>(MainController->GetHUD());
	FInputModeUIOnly UiGameInput;
	MainController->SetInputMode(UiGameInput);
	MainController->DisableInput(MainController);
	MainHUD->AddSelectWeapon();
	//bShowSelectUi = true;
	MainController->bShowMouseCursor = true;
	MainController->bEnableMouseOverEvents = true;

	UpdateHpHUD();
	UpdateStaminaHUD();
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("INITAILAIZING ERROR"));
		return;
	}

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
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(MainController->GetHUD()) : MainHUD;

	FCrosshairPackage HUDPackage;

	HUDPackage.CrosshairCenter = CrosshairsCenter;
	HUDPackage.CrosshairLeft = CrosshairsLeft;
	HUDPackage.CrosshairRight = CrosshairsRight;
	HUDPackage.CrosshairTop = CrosshairsTop;
	HUDPackage.CrosshairBottom = CrosshairsBottom;

	MainHUD->SetHUDPackage(HUDPackage);
}

void ACharacterBase::UpdateHpHUD()
{
	if (MainController)
	{
		MainController->SetHUDHealth(FMath::Clamp(Health,0.f,100.f), MaxHealth);
	}
}

void ACharacterBase::UpdateStaminaHUD()
{
	if (MainController)
	{
		MainController->SetHUDStamina(FMath::Clamp(Stamina, 0.f, 100.f) , MaxStamina);
	}
}

void ACharacterBase::SetWeapon(TSubclassOf<class AWeaponBase> Weapon)
{
	//if (!CurWeapon)
	//{
	AActor* SpawnWeapon = GetWorld()->SpawnActor<AWeaponBase>(Weapon);
	CurWeapon = Cast<AWeaponBase>(SpawnWeapon);

	const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(FName("WeaponSocket"));


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


	if (!IsFalling && Speed == 0.f) //���� �ƴϰ� ��������
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
	if (IsFalling || Speed > 0.f) // �ٰų� �����϶�
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
	//UE_LOG(LogTemp, Log, TEXT("FIRE"));
	if (bCanFire == true)
	{
		bCanFire = false;
		if (CurWeapon) {
			CurWeapon->Fire(HitTarget);
		}
		PlayFireActionMontage(false);

		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraShake(UShoot::StaticClass());

		StartFireTimer();
	}
}

void ACharacterBase::FirePressd(bool _Pressd)
{
	if (_Pressd && CharacterState!= ECharacterState::ECS_SPRINT)
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
			DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
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
	bFirePressed = true;
	FirePressd(bFirePressed);
}
void ACharacterBase::Fire_E(const FInputActionValue& Value)
{
	bFirePressed = false;
	FirePressd(bFirePressed);
}
void ACharacterBase::Inter(const FInputActionValue& Value)
{
	//MainController = Cast<ACharacterController>(Controller);
	//MainHUD = Cast<AMainHUD>(MainController->GetHUD());
	////UE_LOG(LogTemp, Log, TEXT("hahah"));
	//if (bInRespon)
	//{
	//	if (!bShowSelectUi)
	//	{
	//		//UE_LOG(LogTemp, Log, TEXT("TESTTEST"));
	//
	//		FInputModeGameAndUI UiAndGameInput;
	//		MainController->SetInputMode(UiAndGameInput);

	//		MainHUD->AddSelectWeapon();
	//		bShowSelectUi = true;
	//		MainController->bShowMouseCursor = true;
	//		MainController->bEnableMouseOverEvents = true;
	//	}
	//	else
	//	{
	//		//UE_LOG(LogTemp, Log, TEXT("TESTTEST"));
	//		FInputModeGameOnly GameOnlyInput;
	//		MainController->SetInputMode(GameOnlyInput);

	//		MainHUD->RemoveSelectWeapon();
	//		bShowSelectUi = false;
	//		MainController->bShowMouseCursor = false;
	//		MainController->bEnableMouseOverEvents = false;


	//	}
	//}
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
	//UE_LOG(LogTemp, Log, TEXT("%d"), TurningType);
	FHitResult HitResult;
	TraceUnderCrossHiar(HitResult);
	HitTarget = HitResult.ImpactPoint;
	WSADATA WsaData;
	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("INITAILAIZING ERROR"));
		return;
	}

	// ������ IP �ּҿ� ��Ʈ ��ȣ�� �����մϴ�.
	FString ServerIP = TEXT("127.0.0.1");
	int32 ServerPort = 12345; // ������ ��Ʈ ��ȣ

	// ���� ����
	SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ClientSocket == INVALID_SOCKET)
	{
		UE_LOG(LogTemp, Error, TEXT("SOCKET PRODUCE FAILED"));
		WSACleanup(); // ���� �ʱ�ȭ ����
		return;
	}

	// ���� ���� ����
	sockaddr_in ServerAddress;
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = htons(ServerPort);
	ServerAddress.sin_addr.s_addr = inet_addr(TCHAR_TO_UTF8(*ServerIP));

	// ������ ����
	if (connect(ClientSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress)) == SOCKET_ERROR)
	{
		//UE_LOG(LogTemp, Error, TEXT("FAILED."));
		//closesocket(ClientSocket); // ���� �ݱ�
		//WSACleanup(); // ���� �ʱ�ȭ ����
		//return;
	}
	// ĳ������ ���� ��ġ�� ����ϴ�.
	FVector CharacterLocation = GetActorLocation();

	// ��ġ ������ ���ڿ��� ��ȯ�մϴ�.
	FString LocationString = FString::Printf(TEXT("X=%f, Y=%f, Z=%f"), CharacterLocation.X, CharacterLocation.Y, CharacterLocation.Z);
	// ���ڿ��� ����Ʈ �迭�� ��ȯ
	TArray<uint8> SendData;
	FTCHARToUTF8 Convert(*LocationString);
	SendData.Append(reinterpret_cast<const uint8*>(Convert.Get()), Convert.Length());
	// ������ ����
	int32 BytesSent = send(ClientSocket, (const char*)SendData.GetData(), SendData.Num(), 0);
	if (BytesSent == SOCKET_ERROR)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SUCCESS : %s"), *LocationString);
	}

	// ���� �ݱ�
	closesocket(ClientSocket);
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
		EnhancedInputComponent->BindAction(InterAction, ETriggerEvent::Started, this, &ACharacterBase::Inter);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ACharacterBase::Fire_S);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ACharacterBase::Fire_E);
	}
}

