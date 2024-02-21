// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character3.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

ACharacter3::ACharacter3()
{
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));

}
void ACharacter3::BeginPlay()
{
	Super::BeginPlay();
}

void ACharacter3::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("RecordedGhostTime : %f"), RecordedGhostTime));
	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("GhostCoolChargeTime : %f"), GhostCoolChargeTime));
	if (bCoolTimeFinish && bGhost)
	{
		GhostStart();
		RecordedGhostTime += DeltaTime;
		if (RecordedGhostTime >= 5.f) GhostEnd();
	}

	if (!bCoolTimeFinish)
	{
		GhostCoolChargeTime += DeltaTime;
		if (GhostCoolChargeTime >= 10.f)
		{
			bCoolTimeFinish = true;
			GhostCoolChargeTime = 0.f;
		}
	}
}

void ACharacter3::Skill_S(const FInputActionValue& Value)
{
	bGhost = true;
	//NiagaraComp->Activate();
}

void ACharacter3::Skill_E(const FInputActionValue& Value)
{

	bGhost = false;
	GhostCoolChargeTime = false;
	//NiagaraComp->Deactivate();
}

void ACharacter3::GhostStart()
{
	if (bGhost && bCoolTimeFinish)
	{
		OldVelocity = GetMovementComponent()->Velocity;
		GetMovementComponent()->Velocity = GetActorForwardVector() * 2500.f;
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);

	}
}

void ACharacter3::GhostEnd()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMovementComponent()->Velocity = OldVelocity;
	bCoolTimeFinish = false;
	bGhost = false;
	RecordedGhostTime = 0.f;

}
