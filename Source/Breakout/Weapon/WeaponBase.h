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
	void DetectTool(FVector& HitRes);
	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere)
	int32 CurAmmo = 10;

	UPROPERTY(EditAnywhere)
	int32 MaxAmmo = 10;

	FVector StartBeam;
	FVector EndBeam;

	class USoundCue* GetReloadSound() { return ReloadSound; }
protected:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<class USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectileBullet> ProjectileBulletClass;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USpotLightComponent> SpotLight;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> FireSound;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ReloadSound;

	//√—æÀ »Â≈Õ¡¸ ø‰º“µÈ
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 100.f;

	//√—æÀ »Â≈Õ¡¸ ªÁøÎ «“¡ˆ∏ª¡ˆ
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = true;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Firerate=0.15;

public:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Range = 1500.f;
protected:
	virtual void BeginPlay() override;

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	virtual void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);


public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraSystem> BeamNiagara;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraSystem> ImpactNiagara;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraComponent>DetectNiagara;
public:
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	void SetDetectNiagara(bool bUse);
	class USpotLightComponent* GetSpotLight() { return SpotLight; }
};
