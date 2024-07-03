// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InterActor/InterActorBase.h"
#include "ExplosiveObject.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AExplosiveObject : public AInterActorBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay();

	virtual void Destroyed();
public:
	UFUNCTION()
	void ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ExplosionSound;
};
