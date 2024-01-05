// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class BREAKOUT_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:
	AWeaponBase();
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<class USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

protected:
	virtual void BeginPlay() override;



};
