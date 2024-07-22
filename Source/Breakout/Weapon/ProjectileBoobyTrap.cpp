// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBoobyTrap.h"
#include "Kismet/GameplayStatics.h"
#include"Character/CharacterBase.h"
AProjectileBoobyTrap::AProjectileBoobyTrap()
{
	Hp = 15.f;
}

void AProjectileBoobyTrap::BeginPlay()
{
	Super::BeginPlay();
	OnTakeAnyDamage.AddDynamic(this, &AProjectileBoobyTrap::ReciveDamage);
}

void AProjectileBoobyTrap::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NomalImpulse, const FHitResult& Hit)
{
	ACharacterBase* InCh = Cast<ACharacterBase>(OtherActor);
	if (InCh && InCh == GetOwner())
		return;
	else if(InCh)
	{
		UE_LOG(LogTemp, Log, TEXT("BOOM"));
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
		//DrawDebugSphere(GetWorld(), GetActorLocation(), 200.f, 20, FColor::Black, false, 10, 0, 1);
		//DrawDebugSphere(GetWorld(), GetActorLocation(), 500.f, 20, FColor::Purple, false, 10, 0, 1);
		Destroy();
	}
}

void AProjectileBoobyTrap::ReciveDamage(AActor* DamagedActor, float _Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{

	Hp -= _Damage;

	if (Hp <= 0.f)
	{
		Destroy();
	}
}
