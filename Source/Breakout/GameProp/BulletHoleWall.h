// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PropBase.h"
#include "DynamicMesh/DynamicMesh3.h"
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
	//void SetBulletHole(const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable)
	FMeshData MeshBoolean(UPARAM(ref)FMeshData DataA, FTransform TransformA, UPARAM(ref)FMeshData DataB, FTransform TransformB);

	UE::Geometry::FDynamicMesh3 ConvertToFDynamicMesh3(FMeshData& Data);
	FMeshData ConverToFMeshData(UE::Geometry::FDynamicMesh3& Input, FMeshData& Output);

	FTransform3d ConvertToFTransform3d(FTransform Input);
protected:
	virtual void BeginPlay() override;

	TObjectPtr<class USceneComponent> DefaultRoot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Mesh")
	TObjectPtr<class UProceduralMeshComponent> ProceduralMesh;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<class UStaticMeshComponent > Sphere;

	UPROPERTY(BlueprintReadWrite)
	FMeshData MeshDataA;
	UPROPERTY(BlueprintReadWrite)
	FMeshData MeshDataB;

	FVector HitLoc;
	FVector HitNomal;
	FVector DirWorld;

private:

	UFUNCTION(BlueprintCallable)
	void GetMeshDataFromStaticMesh(UStaticMesh* Mesh, UPARAM(ref) FMeshData& Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections);
	UFUNCTION(BlueprintCallable)
	void SetColorData(UPARAM(ref) FMeshData& Data, FLinearColor Color);
	UFUNCTION(BlueprintCallable)
	FMeshData SetRandomVertex(UPARAM(ref)FMeshData& MeshData, float Min, float Max, float Tolerance);
	UFUNCTION(BlueprintCallable)
	FMeshData TransformMeshData(UPARAM(ref) FMeshData& Data, FTransform Transform, bool InPlace, FVector Pivot);
};
