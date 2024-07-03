// Fill out your copyright notice in the Description page of Project Settings.


#include "InterActor/ExplosiveObject.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
void AExplosiveObject::BeginPlay()
{
	Super::BeginPlay();
	OnTakeAnyDamage.AddDynamic(this, &AExplosiveObject::ReciveDamage);
	HP = 30.f;
}

void AExplosiveObject::ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	HP -= Damage;
	if (HP <= 0)
	{
		AController* FiringController = InstigatorController;
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // World context object
				40.f, // BaseDamage
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
		if (ExplosionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this,ExplosionSound, GetActorLocation());
		}
		Destroy();
	}
}

void AExplosiveObject::Destroyed()
{
	Super::Destroyed();
	if (ImpactNiagara)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagara, GetActorLocation());

}
