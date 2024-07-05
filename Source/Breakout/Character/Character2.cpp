// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character2.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ArrowComponent.h"
#include "Player/CharacterController.h"
#include "Game/BOGameInstance.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "../../Server/Server/protocol.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Weapon/WeaponBase.h"
ACharacter2::ACharacter2()
{
	FXroc = CreateDefaultSubobject<UArrowComponent>(TEXT("FXroc"));
	FXroc->SetupAttachment(RootComponent);

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> DashFxRef(TEXT("/Game/Niagara/SKill/Skill2/NS_Skill2.NS_Skill2"));
	NiagaraComp->bAutoActivate = false;
	NiagaraComp->SetAsset(DashFxRef.Object);
	NiagaraComp->SetupAttachment(FXroc);

	MovementComp = GetCharacterMovement();


	OldMaxAcceleration = GetCharacterMovement()->MaxAcceleration;
	OldMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	OldRotationRate = GetCharacterMovement()->RotationRate;

	DashSpeed = 4000.f;

	SetSprint();
}

void ACharacter2::BeginPlay()
{
	Super::BeginPlay();
	inst = Cast<UBOGameInstance>(GetGameInstance());
	DashPoint = 3;
	if (MainController)
	{
		MainController->SetHUDCoolVisibility(true);
		MainController->SetHUDCool(DashPoint);
	}
	
}

void ACharacter2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MainController)
	{
		//GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("DashPoint : %d"), DashPoint));
		if (DashPoint < 3)
			DashCoolChargeTime += DeltaTime;
		if (DashPoint < 3 && DashCoolChargeTime >= 4.f)
		{
			DashPoint += 1;
			MainController->SetHUDCool(DashPoint);
			DashCoolChargeTime = 0.f;
		}
		if (DashPoint == 0)
			MainController->SetHUDSkillOpacity(0.3);
		else
			MainController->SetHUDSkillOpacity(1);
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
		DashSetup(DashSpeed, 100000000.f, FRotator(0.f, 0.f, 10000000.f), false);
		NiagaraComp->Activate();
		bSkillUsing = true;
		//bDash = true;
		//DashStart();
		GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &ACharacter2::DashFinishSetup, 0.2, false);
		//스킬 패킷
		if(inst)
			Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Niagara_packet(_SessionId, PlayerType::Character2, 0);
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
	GetMesh()->SetVisibility(_Visibillity);
	CurWeapon->GetWeaponMesh()->SetVisibility(_Visibillity);
	CanJump = false;
	NiagaraComp->Deactivate();
	//DisableInput(UGameplayStatics::GetPlayerController(GetWorld(),0));
}

void ACharacter2::DashFinishSetup()
{
	DashPoint -= 1;

	MainController->SetHUDCool(DashPoint);

	MovementComp->MaxAcceleration = OldMaxAcceleration;
	MovementComp->MaxWalkSpeed = OldMaxWalkSpeed;
	MovementComp->RotationRate = OldRotationRate;
	GetMesh()->SetVisibility(true);
	CurWeapon->GetWeaponMesh()->SetVisibility(true);
	CanJump = true;
	bSkillUsing = false;
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


void ACharacter2::ServerNiagaraSync()
{
	if (DashPoint > 0 && !GetMovementComponent()->IsFalling())
	{
		DashSetup(DashSpeed, 100000000.f, FRotator(0.f, 0.f, 10000000.f), true);
		NiagaraComp->Activate();
		//bDash = true;
		//DashStart();
		//GetCamera()->bCameraMeshHiddenInGame = true;
		GetMesh()->SetVisibility(false);
		CurWeapon->GetWeaponMesh()->SetVisibility(false);
		//GetMesh()->SetVisibility(false);
		//GetCurWeapon()->GetWeaponMesh()->SetVisibility(false);

		GetWorld()->GetTimerManager().SetTimer(DashTimer, this, &ACharacter2::ServerDashFinish, 0.2, false);

		//스킬 패킷
		
	}
}

void ACharacter2::ServerDashFinish()
{
	if (inst)
		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_CH2_SKILL_PACKET(_SessionId, PlayerType::Character2, true);
	//MovementComp->MaxAcceleration = OldMaxAcceleration;
	//MovementComp->MaxWalkSpeed = OldMaxWalkSpeed;
	//MovementComp->RotationRate = OldRotationRate;
	//GetMesh()->SetHiddenInGame(false, false);

}
