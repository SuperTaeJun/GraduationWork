// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/ProjectileBase.h"
#include "ProjectileBoobyTrap.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AProjectileBoobyTrap : public AProjectileBase
{
	GENERATED_BODY()
public:
	AProjectileBoobyTrap();

protected:
	virtual void BeginPlay() override;

	virtual void OnHit
	(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NomalImpulse, const FHitResult& Hit);


};