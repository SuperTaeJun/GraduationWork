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
	virtual void Fire(const FVector& HitTarget);
	FORCEINLINE float GetFirerate() { return Firerate; }
private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<class USkeletalMeshComponent> WeaponMesh;

	//�Ѿ� ������ ��ҵ�
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 100.f;

	//�Ѿ� ������ ��� ��������
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = true;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Firerate=0.15;
protected:
	virtual void BeginPlay() override;

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	//�Ѿ� �浹������ ��ƼŬ
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	//�Ѿ� ���󰡴� ��ƼŬ
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;
public:
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }


};
