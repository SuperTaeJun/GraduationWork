// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PropBase.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "ProceduralMeshComponent.h"
#include "BulletHoleWall.generated.h"

UCLASS()
class BREAKOUT_API ABulletHoleWall : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletHoleWall();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetBulletHole(const FVector SweepResult);

	TObjectPtr<UProceduralMeshUtility> ProcMeshUtillity;

	bool bUsing = true;

	int32 ID = -1;
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	TObjectPtr<class USceneComponent> DefaultRoot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Mesh")
	TObjectPtr<class UProceduralMeshComponent> ProceduralMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Mesh")
	TObjectPtr<class UStaticMeshComponent > Sphere;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> MeshA;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> MeshB;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMesh> SculptureMesh;

private:
	MeshData MeshDataA;
	MeshData MeshDataB;
	MeshData SculptureData;

	MeshData ResetMeshData;

	FVector HitLoc;
	FVector HitNomal;
	FVector DirWorld;

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UMaterialInstance> CurMaterial;
	TObjectPtr<class UMaterialInstanceDynamic> DynamicMaterial;
	float Hp = 50.f;
	bool bDestroyed = false;
	TArray<MeshData> MeshDataStorage;
	FTimerHandle DestroyTimer;

	//µðÁ¹ºê
	bool bDissolve = false;
	float DissolvePercent = -3.f;
	TObjectPtr<class UMaterialInstanceDynamic> MDynamicDissolveInst;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UMaterialInstance> MDissolveInst;

	TArray<class UProceduralMeshComponent*> MeshSculptures;
	//

	void AllDestroy();
};
