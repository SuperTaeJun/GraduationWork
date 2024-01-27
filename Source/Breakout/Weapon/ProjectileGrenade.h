// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/ProjectileBase.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AProjectileGrenade : public AProjectileBase
{
	GENERATED_BODY()
	
public:
	AProjectileGrenade();
	void StartDestroyTimer();
	void DestroyTimerFinished();
	FTimerHandle DestroyTimer;

	float DestroyTime = 3.f;
protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
};
