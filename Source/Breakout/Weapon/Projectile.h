// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BREAKOUT_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

protected:

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NomalImpulse, const FHitResult& Hit);

	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;


private:
	FTimerHandle DestroyTimer;	
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> CollisionBox;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovementComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UParticleSystem> Tracer;

	TObjectPtr<class UParticleSystemComponent> TracerComponent;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UParticleSystem>ImpactParticles;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ImpactSound;

};
