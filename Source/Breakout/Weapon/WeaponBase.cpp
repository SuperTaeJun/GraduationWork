// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/WeaponBase.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "particles/ParticleSystemComponent.h"
#include "Character/CharacterBase.h"
#include "GameProp/BulletHoleWall.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include"GameProp/EscapeTool.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
//#define TRACE_LENGTH 1000.f

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	BeamNiagara = ConstructorHelpers::FObjectFinder<UNiagaraSystem>(TEXT("/Script/Niagara.NiagaraSystem'/Game/Niagara/Weapon/RifleAndShotgun/NS_Beam.NS_Beam'")).Object;

	DetectNiagara  =CreateDefaultSubobject<UNiagaraComponent>(TEXT("DetectNiagara"));
	DetectNiagara->SetupAttachment(RootComponent);
	DetectNiagara->SetAsset(ConstructorHelpers::FObjectFinder<UNiagaraSystem>(TEXT("/Script/Niagara.NiagaraSystem'/Game/Niagara/Weapon/Detect/NewNiagaraSystem.NewNiagaraSystem'")).Object);
	DetectNiagara->SetAutoActivate(false);

	DetectNiagara->SetWorldRotation(FRotator(90.f, 90.f, 0).Quaternion());
	DetectNiagara->SetWorldLocation(FVector(0.f, 580.f, 0.f));

}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

FVector AWeaponBase::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	//UE_LOG(LogTemp, Log, TEXT("TRACE"));
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLoc = SphereCenter + RandVector;
	FVector ToEndLoc = EndLoc - TraceStart;

	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, false, 0.5f);
	//DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, false, 0.5f);
	//DrawDebugLine(
	//	GetWorld(),
	//	TraceStart,
	//	FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
	//	FColor::Cyan,
	//	false,
	//	0.5f);

	return FVector(TraceStart + ToEndLoc * Range / ToEndLoc.Size());
}

void AWeaponBase::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{

	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel
		(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		//DrawDebugLine
		//(
		//	World,
		//	TraceStart,
		//	End,
		//	FColor::Cyan,
		//	false,
		//	0.5f
		//);

		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		if (BeamNiagara)
		{
			StartBeam = TraceStart;
			EndBeam = BeamEnd;
			UNiagaraComponent* Beam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				BeamNiagara,
				TraceStart,
				WeaponMesh->GetComponentRotation(),
				FVector(1.f),
				true
			);

			if (Beam)
			{
				Beam->SetVectorParameter(FName("End"), BeamEnd);
			}
			Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_AttackPacket(Cast<ACharacterBase>(GetOwner())->_SessionId, StartBeam, EndBeam);
		}
	}
}

void AWeaponBase::SetDetectNiagara(bool bUse)
{
	if (bUse)
	{
		DetectNiagara->Activate();
	}
	else
	{
		DetectNiagara->Deactivate();
	}
}

void AWeaponBase::DetectTool(FVector& HitRes)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleSocket && InstigatorController)
	{
		//UE_LOG(LogTemp, Log, TEXT("TTEST"));
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FHitResult DetectHit;
		TArray<AActor*> DetectOutput;
		UKismetSystemLibrary::SphereTraceSingle
		(
			GetWorld(),
			Start,
			Start + (HitRes - Start) * 0.5f,
			30.f,
			ETraceTypeQuery::TraceTypeQuery1,
			true,
			DetectOutput,
			EDrawDebugTrace::None,
			DetectHit,
			true
		);
		UKismetSystemLibrary::DrawDebugSphere(GetWorld(), Start);
		UKismetSystemLibrary::DrawDebugSphere(GetWorld(), Start+(HitRes - Start)*0.5f);
		if (DetectHit.bBlockingHit)
		{
			if (Cast<AEscapeTool>(DetectHit.GetActor()))
			{
				UE_LOG(LogTemp, Warning, TEXT("DETECT"));
				Cast<AEscapeTool>(DetectHit.GetActor())->SetbDetected(true);

				if (ImpactNiagara)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation
					(
						GetWorld(),
						ImpactNiagara,
						DetectHit.ImpactPoint
					);
				}
			}
		}
	}
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBase::Fire(const FVector& HitTarget)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleSocket /*&& InstigatorController*/)
	{

		UE_LOG(LogTemp, Log, TEXT("TTEST"));
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);


		UWorld* World = GetWorld();
		if (World)
		{
			if (FireHit.bBlockingHit)
			{
				ACharacterBase* DamagedCharacter = Cast<ACharacterBase>(FireHit.GetActor());
				ABulletHoleWall* DamagedWall = Cast<ABulletHoleWall>(FireHit.GetActor());
				if (DamagedCharacter)
				{
					//if (HasAuthority())
					//{
					//	//UE_LOG(LogTemp, Log, TEXT("HIt"));
					//	UGameplayStatics::ApplyDamage(
					//		DamagedCharacter,
					//		Damage,
					//		InstigatorController,
					//		this,
					//		UDamageType::StaticClass()
					//	);
					//}
				}
				else if (DamagedWall)
				{
					DamagedWall->SetBulletHole(FireHit.ImpactPoint);
				}
				if (ImpactNiagara)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation
					(
						World,
						ImpactNiagara,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
					Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_Fire_Effect(Cast<ACharacterBase>(GetOwner())->_SessionId, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}

			}
		}
	
	}
}

