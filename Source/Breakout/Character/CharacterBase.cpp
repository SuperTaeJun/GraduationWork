#include "Character/CharacterBase.h"
//입력
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CameraShake/Sprint.h"
#include "CameraShake/Jog.h"
#include "Kismet/GameplayStatics.h"
ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	CharacterState = ECharacterState::DEFAULT;

	Movement = GetCharacterMovement();
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag;
	CameraBoom->bEnableCameraRotationLag;
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;

	//스테이트
	MaxHealth = 100.f;
	Health = MaxHealth;
	Stamina = 100.f;
	StaminaExhaustionState = false;
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
}

void ACharacterBase::UpdateSprintCamera(float DeltaTime)
{
	if(CameraBoom->TargetArmLength>=300.f && CharacterState == ECharacterState::SPRINT)
		CameraBoom->TargetArmLength -= DeltaTime * 400;
	else if (CameraBoom->TargetArmLength <= 500.f && CharacterState == ECharacterState::RUN)
		CameraBoom->TargetArmLength += DeltaTime * 400;
}
void ACharacterBase::UpdateStamina(float DeltaTime)
{
	if (StaminaExhaustionState == false)
	{
		if (CharacterState == ECharacterState::SPRINT && Stamina >= 0.f)
		{
			//Stamina -= 0.5f;
			if (Stamina <= 0.f)
			{
				StaminaExhaustionState = true;
			}
		}
		else 	if ((CharacterState == ECharacterState::RUN || CharacterState == ECharacterState::IDLE) && Stamina <= 100.f)
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
		CharacterState = ECharacterState::SPRINT;
	}
	else
		CharacterState = ECharacterState::RUN;
}
void ACharacterBase::Sprint_E(const FInputActionValue& Value)
{
	CharacterState = ECharacterState::RUN;
	Movement->MaxWalkSpeed = 400;
}
// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetVelocity().Size() <= 0.f)
		CharacterState = ECharacterState::IDLE;
	switch (CharacterState)
	{
	case ECharacterState::IDLE:
		break;
	case ECharacterState::RUN:
		Movement->MaxWalkSpeed = 400.f;
		bUseControllerRotationYaw = false;
		UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraShake(UJog::StaticClass());
		break;
	case ECharacterState::SPRINT:
		if (StaminaExhaustionState == false)
		{
			Movement->MaxWalkSpeed = 600.f;
			bUseControllerRotationYaw = true;
			UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->StartCameraShake(USprint::StaticClass());
		}
		break;
	case ECharacterState::FALLING:
		break;
	case ECharacterState::DEFAULT:
		Movement->MaxWalkSpeed = 400.f;
		break;
	}
	UpdateStamina(DeltaTime);
	UpdateSprintCamera(DeltaTime);
	UE_LOG(LogTemp, Warning, TEXT("Float : %f "), Stamina);
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharacterBase::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACharacterBase::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ACharacterBase::Sprint_S);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::None, this, &ACharacterBase::Sprint_E);
	}
}

