// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "ProceduralMeshUtility.generated.h"

struct MeshData
{
	MeshData(TArray<FVector> v = {}, TArray<int32> t = {}, TArray<FVector> n = {}, TArray<FVector2D> u = {}, TArray<FLinearColor> c = {}) : Verts(v), Tris(t), Normals(n), UVs(u), Colors(c) {}

	TArray<FVector> Verts = {};
	TArray<int32> Tris = {};
	TArray<FVector> Normals = {};
	TArray<FVector2D> UVs = {};
	TArray<FLinearColor> Colors = {};
	TArray<int32> Sects = {};

	void Clear() 
	{
		Verts = {};
		Tris = {};
		Normals = {};
		UVs = {};
		Colors = {};
		Sects = {};
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
	void GetMeshDataFromStaticMesh(UStaticMesh* Mesh, MeshData& Data, int32 SectionIndex);
	void SetColorData(MeshData& Data, FLinearColor Color);
	void InterpMeshData(MeshData& OutData, MeshData& SourceDataA, MeshData& SourceDataB, float Alpha, bool bClamp);

	MeshData MeshBoolean(MeshData DataA, FTransform TransformA, MeshData DataB, FTransform TransformB, bool OptionType);
	MeshData SetRandomVertex(MeshData& MeshData, float Min, float Max, float Tolerance);
	MeshData TransformMeshData(MeshData& Data, FTransform Transform, bool InPlace, FVector Pivot);
private:
	FVector SpiralCustomLerp(FVector& A, FVector& B, float& Alpha, float SpiralTurns, float Radius);

	UE::Geometry::FDynamicMesh3 ConvertToFDynamicMesh3(MeshData& Data);
	MeshData ConverToMeshData(UE::Geometry::FDynamicMesh3& Input, MeshData& Output);

	FTransform3d ConvertToFTransform3d(FTransform Input);

};
