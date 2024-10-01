// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshUtility.generated.h"

struct MeshData
{
	MeshData(TArray<FVector> v = {}, TArray<int32> t = {}, TArray<FVector> n = {}, TArray<FVector2D> u = {}, TArray<FLinearColor> c = {}) : Verts(v), Tris(t), Normals(n), UVs(u), Colors(c) {}
	UPROPERTY(BlueprintReadWrite)
	TArray<FVector> Verts = {};
	UPROPERTY(BlueprintReadWrite)
	TArray<int32> Tris = {};
	UPROPERTY(BlueprintReadWrite)
	TArray<FVector> Normals = {};
	UPROPERTY(BlueprintReadWrite)
	TArray<FVector2D> UVs = {};
	UPROPERTY(BlueprintReadWrite)
	TArray<FLinearColor> Colors = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> SectSizes = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumSections = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> Sects = {};

	void Clear() {
		Verts = {};
		Tris = {};
		Normals = {};
		UVs = {};
		Colors = {};
		Sects = {};
		SectSizes = {};
		NumSections = 0;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREAKOUT_API UProceduralMeshUtility : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProceduralMeshUtility();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:	
	void UnifyTri(MeshData& Data);
	void GetMeshDataFromStaticMesh(UStaticMesh* Mesh, MeshData& Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections);
	void SetColorData(MeshData& Data, FLinearColor Color);

	void InterpMeshData(MeshData& OutData, MeshData& SourceDataA, MeshData& SourceDataB, float Alpha, bool bClamp);

	FVector SpiralCustomLerp(FVector& A, FVector& B, float& Alpha, float SpiralTurns, float Radius);
};
