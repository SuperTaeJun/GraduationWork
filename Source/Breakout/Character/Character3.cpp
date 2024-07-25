// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character3.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Weapon/WeaponBase.h"
#include "Game/BOGameInstance.h"
#include "Player/CharacterController.h"
#include "ClientSocket.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/PostProcessComponent.h"
#include "Components/SpotLightComponent.h"
ACharacter3::ACharacter3()
{

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraComp->SetAutoActivate(false);
	ConstructorHelpers::FObjectFinder<UNiagaraSystem> FxRef(TEXT("/Game/Niagara/SKill/Skill3/NS_Skill3.NS_Skill3"));
	NiagaraComp->SetAsset(FxRef.Object);
	MovementComp = GetCharacterMovement();

	OldMaxAcceleration = MovementComp->MaxAcceleration;
	OldMaxWalkSpeed = MovementComp->MaxWalkSpeed;

	bCoolTimeFinish = true;

	//OldMaterial = ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Niagara/SKill/Skill3/M_Character3_Inst.M_Character3_Inst'")).Object;

	SetSprint();
	MDissolveInst = ConstructorHelpers::FObjectFinder<UMaterialInstance>(TEXT("/Game/BreakoutAsset/Character/Character3/MI_Character3_Dissolve.MI_Character3_Dissolve")).Object;


}
void ACharacter3::BeginPlay()
{
	Super::BeginPlay();

	inst = Cast<UBOGameInstance>(GetGameInstance());
	//DynamicMaterial = UMaterialInstanceDynamic::Create(OldMaterial, this);
	//if (DynamicMaterial)
	//{
	//	GetMesh()->SetMaterial(0, DynamicMaterial);
	//	DynamicMaterial->SetScalarParameterValue(FName("Alpha"), 0.f);
	//}
	if(MainController)
		MainController->SetHUDCoolVisibility(false);
}

void ACharacter3::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Blue, FString::Printf(TEXT("GhostCoolChargeTime : %f"), GhostCoolChargeTime));

	if (!bCoolTimeFinish && MainController)
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

	//if (DynamicMaterial && bGhost)
	//{
	//	//머터리얼 파라미터 적용
	//	DynamicMaterial->SetVectorParameterValue(FName("Loc"), GetCapsuleComponent()->GetForwardVector() * -1.f);
	//	DynamicMaterial->SetScalarParameterValue(FName("Amount"), GetCharacterMovement()->Velocity.Length() / 4);
	//}
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
	if(CurWeapon)
		GhostStart();
	////패킷 
	//if (inst)
	//	Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Niagara_packet(_SessionId, PlayerType::Character3, 0);
}

void ACharacter3::Skill_E(const FInputActionValue& Value)
{
	if (CurWeapon)
	{
		GhostEnd();
		//패킷
		//if (inst)
		//	Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Niagara_cancel(bCancel, _SessionId, 1);
	}
	
}

void ACharacter3::GhostStart()
{
	if (bCoolTimeFinish)
	{
		bGhost = true;
		bCoolTimeFinish = false;
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
		NiagaraComp->Activate();

		GetMesh()->SetVisibility(false);
		CurWeapon->GetWeaponMesh()->SetVisibility(false);

		//MovementComp->MaxWalkSpeed = 1500;
		//MovementComp->MaxAcceleration = 10000000.f;

		//패킷 
		if (inst)
			Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Niagara_packet(inst->GetPlayerID(), PlayerType::Character3, 0);
		GetWorld()->GetTimerManager().SetTimer(GhostTimer, this, &ACharacter3::GhostEnd, 5.f, false);

	}
}

void ACharacter3::GhostEnd()
{
	if (bGhost)
	{
		MainController->SetHUDCoolVisibility(true);
		MainController->SetHUDSkillOpacity(0.3);
		bGhost = false;
		bCoolTimeFinish = false;

		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

		NiagaraComp->Deactivate();
		GetMesh()->SetVisibility(true, true);
		CurWeapon->GetWeaponMesh()->SetVisibility(true);

		if (bCurLight)
			CurWeapon->GetSpotLight()->SetVisibility(true);
		else
			CurWeapon->GetSpotLight()->SetVisibility(false);

		//MovementComp->MaxWalkSpeed = OldMaxWalkSpeed;
		//MovementComp->MaxAcceleration = OldMaxAcceleration;
		if (inst)
			Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Niagara_cancel(bCancel, inst->GetPlayerID(), 1);
	}
}

void ACharacter3::ServerGhostStart()
{

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	NiagaraComp->Activate();
	GetMesh()->SetVisibility(false);
	CurWeapon->GetWeaponMesh()->SetVisibility(false);

	//MovementComp->MaxWalkSpeed = 1500;
	//MovementComp->MaxAcceleration = 10000000.f;

	GetWorld()->GetTimerManager().SetTimer(GhostTimer, this, &ACharacter3::GhostEnd, 5.f, false);
}

void ACharacter3::ServerGhostEnd()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

	NiagaraComp->Deactivate();
	GetMesh()->SetVisibility(true, true);
	CurWeapon->GetWeaponMesh()->SetVisibility(true);

	if (bCurLight)
		CurWeapon->GetSpotLight()->SetVisibility(true);
	else
		CurWeapon->GetSpotLight()->SetVisibility(false);


	//MovementComp->MaxWalkSpeed = OldMaxWalkSpeed;
	//MovementComp->MaxAcceleration = OldMaxAcceleration;

}

