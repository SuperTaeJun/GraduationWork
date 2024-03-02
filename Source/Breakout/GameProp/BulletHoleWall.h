// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PropBase.h"
#include "BulletHoleWall.generated.h"

//struct FMeshData
//{
//	//GENERATED_BODY()
//
//	FMeshData(TArray<FVector> v = {}, TArray<int32> t = {}, TArray<FVector> n = {}, TArray<FVector2D> u = {}, TArray<FLinearColor> c = {}) : Verts(v), Tris(t), Normals(n), UVs(u), Colors(c) {}
//
//	TArray<FVector> Verts = {};
//	TArray<int32> Tris = {};
//	TArray<FVector> Normals = {};
//	TArray<FVector2D> UVs = {};
//	TArray<FLinearColor> Colors = {};
//
//	UPROPERTY(EditAnywhere)
//	TArray<int32> SectSizes = {};
//	UPROPERTY(EditAnywhere)
//	int32 NumSections = 0;
//	UPROPERTY(EditAnywhere)
//	TArray<int32> Sects = {};
//
//	FORCEINLINE void Clear() {
//		Verts = {};
//		Tris = {};
//		Normals = {};
//		UVs = {};
//		Colors = {};
//		Sects = {};
//		SectSizes = {};
//		NumSections = 0;
//	}
//
//	FORCEINLINE void CountSections() {
//		int sum = 0;
//		for (const int& i : SectSizes) {
//			sum += i;
//		}
//		int vl = Verts.Num();
//		if (sum != vl && Sects.Num() >= vl) {
//			int x = 0, n = 0;
//			SectSizes = {};
//			for (x = 0; x < vl; ++x) {
//				const int& s = Sects[x];
//				while (s >= n) { SectSizes.Add(0); ++n; }
//				++SectSizes[s];
//			}
//			NumSections = n;
//			if (NumSections <= 0) { NumSections = 1; }
//		}
//	}
//
//};


UCLASS()
class BREAKOUT_API ABulletHoleWall : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletHoleWall();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<class UProceduralMeshComponent> ProceduralMesh;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<class UStaticMesh> MeshA;
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<class UStaticMesh> MeshB;

	FMeshData MeshDataA;
	FMeshData MeshDataB;
	TArray<FMeshData> ProcMeshData;

	FVector HitLoc;
	FVector HitNomal;

private:
	void GetMeshDataFromStaticMesh(UStaticMesh* Mesh, FMeshData& Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections);
	void SetColorData(FMeshData& Data, FLinearColor Color);
};
