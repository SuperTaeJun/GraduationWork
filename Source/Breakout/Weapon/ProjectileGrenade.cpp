// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->InitialSpeed = 1500.f;
	ProjectileMovementComponent->MaxSpeed = 1500.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;
	ProjectileMovementComponent->Bounciness = 0.5;
	ProjectileMovementComponent->Friction = 0.3;

	DestroyTime = 5.f;
}

void AProjectileGrenade::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectileGrenade::DestroyTimerFinished,
		DestroyTime
	);
}

void AProjectileGrenade::DestroyTimerFinished()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // World context object
				Damage, // BaseDamage
				10.f, // MinimumDamage
				GetActorLocation(), // Origin
				200.f, // DamageInnerRadius
				500.f, // DamageOuterRadius
				1.f, // DamageFalloff
				UDamageType::StaticClass(), // DamageTypeClass
				TArray<AActor*>(), // IgnoreActors
				this, // DamageCauser
				FiringController // InstigatorController
			);
		}
	}
	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagara, GetActorLocation(),FRotator::ZeroRotator,FVector(150.f,150.f,150.f));
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), 200.f, 20, FColor::Black, false, 10, 0, 1);
	DrawDebugSphere(GetWorld(), GetActorLocation(), 500.f, 20, FColor::Purple, false, 10, 0, 1);
	Destroy();
}

void AProjectileGrenade::BeginPlay()
{
	Super::BeginPlay();

	StartDestroyTimer();
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);

}

//바운스할때마다 발생 나중에 사운드 추가
void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
}
