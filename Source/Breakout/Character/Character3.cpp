// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character3.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Weapon/WeaponBase.h"
#include "Player/CharacterController.h"

#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
ACharacter3::ACharacter3()
{
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetAutoActivate(false);

	MovementComp = GetCharacterMovement();

	OldMaxAcceleration = MovementComp->MaxAcceleration;
	OldMaxWalkSpeed = MovementComp->MaxWalkSpeed;

	bCoolTimeFinish = true;

}
void ACharacter3::BeginPlay()
{
	Super::BeginPlay();
	DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);

	MainController->SetHUDCoolVisibility(false);
}

void ACharacter3::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("GhostCoolChargeTime : %f"), GhostCoolChargeTime));

	if (!bCoolTimeFinish)
	{
		GhostCoolChargeTime += DeltaTime;
		MainController->SetHUDCool(GhostCoolChargeTime,10.f);
		if (GhostCoolChargeTime >= 10.f)
		{
			bCoolTimeFinish = true;
			GhostCoolChargeTime = 0.f;
			MainController->SetHUDCoolVisibility(false);
			MainController->SetHUDSkillOpacity(1.f);

		}
	}

	//머터리얼 파라미터 적용
	DynamicMaterial->SetVectorParameterValue(FName("Loc"), GetCapsuleComponent()->GetForwardVector()*-1.f);
	DynamicMaterial->SetScalarParameterValue(FName("Amount"), GetCharacterMovement()->Velocity.Length()/4);
}

void ACharacter3::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &ACharacter3::Skill_S);
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Completed, this, &ACharacter3::Skill_E);
	}
}

void ACharacter3::Skill_S(const FInputActionValue& Value)
{
	GhostStart();
}

void ACharacter3::Skill_E(const FInputActionValue& Value)
{
	GhostEnd();
}

void ACharacter3::GhostStart()
{
	if (bCoolTimeFinish)
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
		MovementComp->MaxWalkSpeed = 1500;
		MovementComp->MaxAcceleration = 10000000.f;
		NiagaraComp->Activate();

		GetMesh()->SetVisibility(true, true);
		CurWeapon->GetWeaponMesh()->SetVisibility(false);
		GetMesh()->SetMaterial(0, DynamicMaterial);

		GetWorld()->GetTimerManager().SetTimer(GhostTimer, this, &ACharacter3::GhostEnd, 4.f, false);
	}
}

void ACharacter3::GhostEnd()
{
	MainController->SetHUDCoolVisibility(true);
	MainController->SetHUDSkillOpacity(0.3);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	MovementComp->MaxWalkSpeed = OldMaxWalkSpeed;
	MovementComp->MaxAcceleration = OldMaxAcceleration;
	NiagaraComp->Deactivate();
	GetMesh()->SetVisibility(true, true);
	CurWeapon->GetWeaponMesh()->SetVisibility(true);
	bCoolTimeFinish = false;
	GetMesh()->SetMaterial(0, OldMaterial);
}

