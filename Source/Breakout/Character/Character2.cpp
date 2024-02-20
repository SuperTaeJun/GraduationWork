// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character2.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

ACharacter2::ACharacter2()
{
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> DashFxRef(TEXT("/Game/Niagara/DashFX.DashFX"));
	NiagaraComp->bAutoActivate = false;
	NiagaraComp->SetAsset(DashFxRef.Object);
}

void ACharacter2::BeginPlay()
{
	Super::BeginPlay();
	DashPoint = 3;
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

void ACharacter2::Skill_S(const FInputActionValue& Value)
{
	if (DashPoint > 0)
	{
		bDash = true;
		DashStart();
		NiagaraComp->Activate();
	}
}

void ACharacter2::Skill_E(const FInputActionValue& Value)
{
	NiagaraComp->Deactivate();
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
