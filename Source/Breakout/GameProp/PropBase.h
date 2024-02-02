// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshDescription.h"
#include "PropBase.generated.h"


struct FMeshData
{

	FMeshData(TArray<FVector> v = {}, TArray<int32> t = {}, TArray<FVector> n = {}, TArray<FVector2D> u = {}, TArray<FLinearColor> c = {}) : Verts(v), Tris(t), Normals(n), UVs(u), Colors(c) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshOps")
	TArray<FVector> Verts = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshOps")
	TArray<int32> Tris = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshOps")
	TArray<FVector> Normals = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshOps")
	TArray<FVector2D> UVs = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshOps")
	TArray<FLinearColor> Colors = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshOps")
	float Time = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeshOps")
	bool Updated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	FVector Min = FVector(0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	FVector Max = FVector(0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	FVector Bounds = FVector(0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	FVector Center = FVector(0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	float Radius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	TArray<FVector> StaticVerts = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	TArray<FVector> StaticNormals = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	bool Skinned = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	TArray<int32> SectSizes = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	int32 NumSections = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	TArray<int32> Sects = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	TArray<int32> Parts = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	int32 Partitions = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "MeshOps")
	float PartitionPrecision = 10000.0f;

	float Seglen = 0.0f;

	FORCEINLINE void Clear() {
		Verts = {};
		Tris = {};
		Normals = {};
		UVs = {};
		Colors = {};
		Sects = {};
		SectSizes = {};
		NumSections = 0;
		Time = 0.0f;
		Updated = false;
		Min = FVector(0.0f, 0.0f, 0.0f);
		Max = FVector(0.0f, 0.0f, 0.0f);
		Bounds = FVector(0.0f, 0.0f, 0.0f);
		Center = FVector(0.0f, 0.0f, 0.0f);
		Radius = 0.0f;
		StaticVerts = {};
		StaticNormals = {};
		Skinned = false;
		Seglen = 0.0f;
	}

	FORCEINLINE void CountSections() {
		int sum = 0;
		for (const int& i : SectSizes) {
			sum += i;
		}
		int vl = Verts.Num();
		if (sum != vl && Sects.Num() >= vl) {
			int x = 0, n = 0;
			SectSizes = {};
			for (x = 0; x < vl; ++x) {
				const int& s = Sects[x];
				while (s >= n) { SectSizes.Add(0); ++n; }
				++SectSizes[s];
			}
			NumSections = n;
			if (NumSections <= 0) { NumSections = 1; }
		}
	}

};

UCLASS()
class BREAKOUT_API APropBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APropBase();
	virtual void Tick(float DeltaTime) override;
	void SetHideMesh();
protected:
	virtual void BeginPlay() override;

	void GetMeshDataFromStaticMesh(class UStaticMesh* Mesh, FMeshData* Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections);
	//void UnifyTri(FMeshData& Data);
	//void SplitVertexes(FMeshData& Data);
	//void TransformMeshData(FMeshData& Data, FTransform Transform, FVector Pivot);


	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	void AddTriangle(int32 V1, int32 V2, int32 V3);

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<class UProceduralMeshComponent> ProceduralMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<class USphereComponent>AreaSphere;


	FMeshData Data1;
	FMeshData Data2;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<UStaticMesh> Mesh1;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<UStaticMesh> Mesh2;


	void ProceduralMeshFromMeshData(UProceduralMeshComponent* Mesh, FMeshData& Data, int SectionIndex, bool Collision, bool CalcTangents);
	TArray<FMeshData> ConvertFromSectionedMeshData(FMeshData& Data);
};
