// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/ProjectileBase.h"
#include "ProjectileWall.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AProjectileWall : public AProjectileBase
{
	GENERATED_BODY()
public:
	AProjectileWall();
	
protected:
	virtual void BeginPlay() override;


	virtual void OnHit
	(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NomalImpulse, const FHitResult& Hit) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABulletHoleWall> WallClass;
	class UBOGameInstance* inst;
};
