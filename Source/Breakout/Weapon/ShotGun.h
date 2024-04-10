// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponBase.h"
#include "ShotGun.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AShotGun : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitTarget) override;
	void ShotGunTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit ,FVector& EndBeamLoc);
private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10;
};
