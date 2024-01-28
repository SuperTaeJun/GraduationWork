// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/WeaponBase.h"
#include "RocketLauncher.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ARocketLauncher : public AWeaponBase
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectileBase> ProjectileClass;
};
