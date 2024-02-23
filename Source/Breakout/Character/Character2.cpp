// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character2.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ArrowComponent.h"

#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
ACharacter2::ACharacter2()
{
	FXroc = CreateDefaultSubobject<UArrowComponent>(TEXT("FXroc"));
	FXroc->SetupAttachment(RootComponent);

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> DashFxRef(TEXT("/Game/Niagara/Skill2.Skill2"));
	NiagaraComp->bAutoActivate = false;
	NiagaraComp->SetAsset(DashFxRef.Object);
	NiagaraComp->SetupAttachment(FXroc);

	MovementComp = GetCharacterMovement();


	OldMaxAcceleration = GetCharacterMovement()->MaxAcceleration;
	OldMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	OldRotationRate = GetCharacterMovement()->RotationRate;

	DashSpeed = 4000.f;


}

void ACharacter2::BeginPlay()
{
	Super::BeginPlay();
	DashPoint = 3;

	if (MovementComp)
	{

	}
}

void ACharacter2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("DashPoint : %d"), DashPoint));
	if (DashPoint < 3) DashCoolChargeTime += DeltaTime;
	if (DashPoint < 3 && DashCoolChargeTime >= 4.f)
	{
		DashPoint += 1;
		DashCoolChargeTime = 0.f;
	}
}

void ACharacter2::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &ACharacter2::Skill_S);
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Completed, this, &ACharacter2::Skill_E);
	}
}

void ACharacter2::Skill_S(const FInputActionValue& Value)
{
	if (DashPoint > 0 && !GetMovementComponent()->IsFalling())
	{
		DashSetup(DashSpeed, 100000000.f, FRotator(0.f, 0.f, 10000000.f), true);
		NiagaraComp->Activate();
		//bDash = true;
		//DashStart();
		GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &ACharacter2::DashFinishSetup, 0.2, false);
	}
}

void ACharacter2::Skill_E(const FInputActionValue& Value)
{

}

void ACharacter2::DashSetup(float _MaxWalk, float _MaxAcc, FRotator _Rotation ,bool _Visibillity)
{
	MovementComp->MaxAcceleration = _MaxAcc;
	MovementComp->MaxWalkSpeed = _MaxWalk;
	MovementComp->RotationRate = _Rotation;
	GetMesh()->SetHiddenInGame(_Visibillity, true);
	CanJump = false;
	NiagaraComp->Deactivate();
	//DisableInput(UGameplayStatics::GetPlayerController(GetWorld(),0));
}

void ACharacter2::DashFinishSetup()
{
	MovementComp->MaxAcceleration = OldMaxAcceleration;
	MovementComp->MaxWalkSpeed = OldMaxWalkSpeed;
	MovementComp->RotationRate = OldRotationRate;
	GetMesh()->SetHiddenInGame(false, true);
	CanJump = true;
	//EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}

void ACharacter2::DashStart()
{
	if (bDash && bCoolTimeFinish)
	{
		//UE_LOG(LogTemp, Log, TEXT("DASHPOINT --"));
		bDash = false;
		bCoolTimeFinish = false;
		DashPoint -= 1;
		OldVelocity = GetMovementComponent()->Velocity;
		GetMovementComponent()->Velocity = //ForwardDir * 5000.f;
			GetActorForwardVector() * 20000.f;
		GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &ACharacter2::FinishDashTimer, 0.5, false);
	}
}

void ACharacter2::FinishDashTimer()
{
	GetMovementComponent()->Velocity = OldVelocity;
	GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &ACharacter2::CoolTimeDashTimer, 0.2, false);
}

void ACharacter2::CoolTimeDashTimer()
{
	bCoolTimeFinish = true;
}
