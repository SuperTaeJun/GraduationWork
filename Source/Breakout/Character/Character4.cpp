// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character4.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
ACharacter4::ACharacter4()
{
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetAutoActivate(false);
	NiagaraComp2 = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp2"));
	NiagaraComp2->SetAutoActivate(false);
}
void ACharacter4::BeginPlay()
{
	Super::BeginPlay();
	TelepoChargeTime = true;
}

void ACharacter4::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("RecordedTelpoTime : %f"), CoolChargeTime));
	if (!TelepoChargeTime)
	{
		CoolChargeTime += DeltaTime;
		if (CoolChargeTime >= 15.f)
		{
			TelepoChargeTime = true;
		}
	}
}

void ACharacter4::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &ACharacter4::Skill_S);
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Completed, this, &ACharacter4::Skill_E);
	}
}

void ACharacter4::Skill_S(const FInputActionValue& Value)
{
	if (!bSaved&& TelepoChargeTime)
	{
		NiagaraComp2->Activate();
		SaveCurLocation();
		//GhostMesh = GetWorld()->SpawnActor<AReplayFX>(AReplayFX::StaticClass(), GetActorTransform());
		//GhostMesh->Init(GetMesh());
		//UE_LOG(LogTemp, Log, (TEXT("TEST")));
	}
	else if(bSaved)
	{
		NiagaraComp->Activate();
		GetMesh()->SetVisibility(false, true);
		GetWorld()->GetTimerManager().SetTimer(TelpoTimer, this, &ACharacter4::SetLocation, 0.5f, false);
	}
}

void ACharacter4::Skill_E(const FInputActionValue& Value)
{

}

void ACharacter4::SaveCurLocation()
{

	SavedLocation = GetActorLocation();
	bSaved = true;
	
}

void ACharacter4::SetLocation()
{

	bSaved = false;
	TelepoChargeTime = false;
	SetActorLocation(SavedLocation);

	NiagaraComp->Deactivate();
	GetMesh()->SetVisibility(true, true);
}
