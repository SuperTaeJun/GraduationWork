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
	void SetBulletHole(const FHitResult& SweepResult);
	FMeshData MeshBoolean(FMeshData DataA, FTransform TransformA, FMeshData DataB, FTransform TransformB);
	UE::Geometry::FDynamicMesh3 ConvertToFDynamicMesh3(FMeshData& Data);
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<class UProceduralMeshComponent> ProceduralMesh;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<class UBoxComponent > CollisionBox;

	FMeshData MeshDataA;
	FMeshData MeshDataB;
	TArray<FMeshData> ProcMeshData;

	FVector HitLoc;
	FVector HitNomal;


private:
	void GetMeshDataFromStaticMesh(UStaticMesh* Mesh, FMeshData& Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections);
	void SetColorData(FMeshData& Data, FLinearColor Color);
};
