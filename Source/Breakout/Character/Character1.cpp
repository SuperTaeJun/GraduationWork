// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character1.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Player/CharacterController.h"
#include "Game/BOGameMode.h"

ACharacter1::ACharacter1()
{
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> DashFxRef(TEXT("/Game/Niagara/DashFX.DashFX"));
	NiagaraComp->bAutoActivate = false;
	NiagaraComp->SetAsset(DashFxRef.Object);

	bCoolTimeFinish = true;
}

void ACharacter1::BeginPlay()
{
	Super::BeginPlay();
	if(MainController)
		MainController->SetHUDCoolVisibility(false);
	MaxSaveTime = 5.f;
}
void ACharacter1::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("RecordedTime : %f"), RecordedTime));
	if (!bCoolTimeFinish && MainController)
	{
		RecordedCoolTime += DeltaTime;
		UpdateHUDCool(RecordedCoolTime,15.f);

		if(RecordedCoolTime >=15.f)
		{
			bCoolTimeFinish = true;
			RecordedCoolTime = 0.f;
			MainController->SetHUDCoolVisibility(false);
			MainController->SetHUDSkillOpacity(1.f);
		}
	}
	if (!bTimeReplay)
	{
		StoreFrameData(DeltaTime);
	}
	else if (!bOutOfData)
	{
		Replay(DeltaTime);
	}

}
void ACharacter1::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &ACharacter1::Skill_S);
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Completed, this, &ACharacter1::Skill_E);
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Triggered, this, &ACharacter1::Skill_T);
	}
}
void ACharacter1::Destroyed()
{
	Super::Destroyed();

}
void ACharacter1::Skill_S(const FInputActionValue& Value)
{
	/*if (bCoolTimeFinish)
	{*/
		bTimeReplay = true;
		GetMesh()->SetHiddenInGame(true, true);
		NiagaraComp->Activate();
		//NiagaraComp->Activate();
	//	}
}

void ACharacter1::Skill_E(const FInputActionValue& Value)
{
	bTimeReplay = false;
	NiagaraComp->Deactivate();
	GetMesh()->SetHiddenInGame(false, true);
	bCoolTimeFinish = false;

	MainController->SetHUDCoolVisibility(true);
	MainController->SetHUDSkillOpacity(0.3);
}

void ACharacter1::Skill_T(const FInputActionValue& Value)
{
	if (bCoolTimeFinish)
	{
	}
}


//Skill1
void ACharacter1::StoreFrameData(float DeltaTime)
{
	RunningTime = 0.f;
	LeftRunningTime = 0.f;
	RightRunningTime = 0.f;
	Temp += DeltaTime;
	FCharacterFrameData Package(GetActorLocation(), Temp);
	if (Temp >= 0.2f)
	{
		if (RecordedTime < MaxSaveTime)
		{
			UE_LOG(LogTemp, Log, TEXT("RECORDETIME"));
			FrameDatas.AddTail(Package);
			RecordedTime += Package.DeltaTime;
			bOutOfData = false;
		}
		else
		{
			while (RecordedTime >= MaxSaveTime)
			{
				auto Head = FrameDatas.GetHead();
				float HeadDeltaTime = Head->GetValue().DeltaTime;
				FrameDatas.RemoveNode(Head);
				RecordedTime -= HeadDeltaTime;
			}
			FrameDatas.AddTail(Package);
			RecordedTime += Package.DeltaTime;
			bOutOfData = false;
		}
		Temp = 0.f;
	}
}
void ACharacter1::Replay(float DeltaTime)
{
	RunningTime += DeltaTime * 4;
	auto Right = FrameDatas.GetTail();
	auto Left = Right->GetPrevNode();
	LeftRunningTime = RightRunningTime + Right->GetValue().DeltaTime;

	while (RunningTime > LeftRunningTime)
	{
		RightRunningTime += Right->GetValue().DeltaTime;
		Right = Left;
		LeftRunningTime += Left->GetValue().DeltaTime;
		Left = Left->GetPrevNode();

		auto Tail = FrameDatas.GetTail();
		RecordedTime -= Tail->GetValue().DeltaTime;
		FrameDatas.RemoveNode(Tail);
		if (Left == FrameDatas.GetHead())
		{
			bOutOfData = true;
		}
	}
	//각수치들 보간
	if (RunningTime <= LeftRunningTime && RunningTime >= RightRunningTime)
	{
		float DT = RunningTime - RightRunningTime;
		float Interval = LeftRunningTime - RightRunningTime;
		float Fraction = DT / Interval;

		FVector InterpLocation = FMath::VInterpTo(Right->GetValue().Location, Left->GetValue().Location, Fraction, 1.f);

		SetActorLocation(InterpLocation);
	}
}

void ACharacter1::UpdateHUDCool(float Cool, float MaxCool)
{
	MainController->SetHUDCool(Cool, MaxCool);
}
