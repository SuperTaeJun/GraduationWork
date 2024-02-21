// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character4.h"
#include "FX/ReplayFX.h"
void ACharacter4::BeginPlay()
{
	Super::BeginPlay();

	Toggle = 1;
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

void ACharacter4::Skill_S(const FInputActionValue& Value)
{
	if (Toggle % 2 == 1)
	{
		SaveCurLocation();
		//GhostMesh = GetWorld()->SpawnActor<AReplayFX>(AReplayFX::StaticClass(), GetActorTransform());
		//GhostMesh->Init(GetMesh());
		//UE_LOG(LogTemp, Log, (TEXT("TEST")));
	}
	else if(Toggle %2 ==0)
	{
		SetLocation();
	}
}

void ACharacter4::Skill_E(const FInputActionValue& Value)
{
	//if (Toggle % 2 == 1)
	//{
	//	GhostMesh->Init(GetMesh());
	//	UE_LOG(LogTemp, Log, (TEXT("TEST")));
	//}
}

void ACharacter4::SaveCurLocation()
{
	if (TelepoChargeTime)
	{
		UE_LOG(LogTemp, Log, TEXT("START"));
		SavedLocation = GetActorLocation();
		bSaved = true;
		GetWorld()->GetTimerManager().SetTimer(TelpoTimer, this, &ACharacter4::SetCanTelepo, 1, false);
		TelepoChargeTime = false;
		//UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),);
	}
}

void ACharacter4::SetLocation()
{
	if (bSaved && CanTelepo)
	{
		UE_LOG(LogTemp, Log, TEXT("End"));
		bSaved = false;
		SetActorLocation(SavedLocation);
		Toggle += 1;
	}
}
