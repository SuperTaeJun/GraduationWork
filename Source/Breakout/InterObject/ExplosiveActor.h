// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InterObject/InterActorBase.h"
#include "ExplosiveActor.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AExplosiveActor : public AInterActorBase
{
	GENERATED_BODY()
	
public:
	AExplosiveActor();

protected:
	virtual void BeginPlay();

	virtual void Destroyed();

	UFUNCTION()
	void ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	UFUNCTION()
	virtual void OnBeginSphereOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	UFUNCTION()
	virtual void OnEndSphereOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ExplosionSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USphereComponent> CollisionSphere;

	class ACharacterBase* InCharacter = nullptr;
};
