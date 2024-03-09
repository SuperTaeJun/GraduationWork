// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/BulletHoleWall.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
#include "Operations/MeshBoolean.h"

//using namespace UE::Geometry;

ABulletHoleWall::ABulletHoleWall()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProceduralMesh->SetupAttachment(CollisionBox);

	ConstructorHelpers::FObjectFinder<UStaticMesh> SMMesh1(TEXT("/Game/Maps/_GENERATED/TAEJUN/Box_24A10B9B.Box_24A10B9B"));
	GetMeshDataFromStaticMesh(SMMesh1.Object, MeshDataA, 0, 0, true);
	
	//GetMeshDataFromStaticMesh(MeshB, MeshDataB, 0, 0, true);
	

	SetColorData(MeshDataA, FLinearColor::Red);

	TArray<FProcMeshTangent> Tangents = {};
	ProceduralMesh->CreateMeshSection_LinearColor
	(
		0,  //SECK INDEX
		MeshDataA.Verts,  //V
		MeshDataA.Tris, //T
		MeshDataA.Normals, //N
		MeshDataA.UVs, //UV
		MeshDataA.Colors, //C
		Tangents, //T
		true // COLLISION
	);
}

void ABulletHoleWall::BeginPlay()
{
	Super::BeginPlay();

}
void ABulletHoleWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABulletHoleWall::SetBulletHole(const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("BulletHole"));
	UE_LOG(LogTemp, Warning, TEXT("LOCATION : %s"), *SweepResult.Location.ToString());

	//UE::Geometry::FDynamicMesh3 DMeshWall = UE::Geometry::mesh2
	//UE::Geometry::FMeshBoolean Boolean()

}

FMeshData ABulletHoleWall::MeshBoolean(FMeshData DataA, FTransform TransformA, FMeshData DataB, FTransform TransformB)
{
	UE::Geometry::FDynamicMesh3 BooleanDynamicMesh;
	BooleanDynamicMesh.EnableAttributes();
	BooleanDynamicMesh.EnableVertexColors(FVector3f(0.0f, 0.0f, 0.0f));
	BooleanDynamicMesh.EnableVertexNormals(FVector3f(0.0f, 0.0f, 0.0f));
	BooleanDynamicMesh.EnableVertexUVs(FVector2f(0.0f, 0.0f));

	UE::Geometry::FMeshBoolean::EBooleanOp Option;
	Option= UE::Geometry::FMeshBoolean::EBooleanOp::Difference;
	UE::Geometry::FDynamicMesh3 DMeshA = ConvertToFDynamicMesh3(DataA);
	UE::Geometry::FDynamicMesh3 DMeshB = ConvertToFDynamicMesh3(DataB);


	//UE::Geometry::FMeshBoolean Boolean();

	return FMeshData();
}

UE::Geometry::FDynamicMesh3 ABulletHoleWall::ConvertToFDynamicMesh3(FMeshData& Data)
{
	UE::Geometry::FDynamicMesh3 Result;
	Result.EnableAttributes();
	Result.EnableVertexColors(FVector3f(0.0f, 0.0f, 0.0f));
	Result.EnableVertexNormals(FVector3f(0.0f, 0.0f, 1.0f));
	Result.EnableVertexUVs(FVector2f(0.0f, 0.0f));

	FVector3d ResultVertex;
	FVector3f ResultNomal;
	FVector2f ResultUVs;
	FVector3f ResultColor;
	int VertexID = 0;
	for (int x = 0; x < Data.Verts.Num(); ++x)
	{
		//버텍스 정보 추가
		ResultVertex.X = Data.Verts[x].X;
		ResultVertex.Y = Data.Verts[x].Y;
		ResultVertex.Z = Data.Verts[x].Z;
		VertexID = Result.AppendVertex(ResultVertex);

		//노멀정보 추가
		ResultNomal.X = Data.Normals[x].X;
		ResultNomal.Y = Data.Normals[x].Y;
		ResultNomal.Z = Data.Normals[x].Z;
		if (x < Data.Normals.Num()) Result.SetVertexNormal(VertexID, ResultNomal);

		//UV정보 추가
		ResultUVs.X = Data.UVs[x].X;
		ResultUVs.Y = Data.UVs[x].Y;
		if (x < Data.UVs.Num()) Result.SetVertexUV(VertexID, ResultUVs);

		//색정보 추가
		ResultColor.X = Data.Colors[x].R;
		ResultColor.Y = Data.Colors[x].G;
		ResultColor.Z = Data.Colors[x].B;
		if (x < Data.Colors.Num()) Result.SetVertexColor(VertexID, ResultColor);

	}
	UE::Geometry::FIndex3i ResultTri;
	for (int x = 0; x < Data.Verts.Num(); x+=3)
	{
		if (x + 2 < Data.Tris.Num())
		{
			ResultTri.A = Data.Tris[x];
			ResultTri.B = Data.Tris[x+1];
			ResultTri.C = Data.Tris[x+2];

			Result.AppendTriangle(ResultTri);
		}
	}
	return Result;
}



void ABulletHoleWall::GetMeshDataFromStaticMesh(UStaticMesh* Mesh, FMeshData& Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections)
{
	int32 n = 0, svi = 0, vi = 0, sec = 0;
	int32* NewIndexPtr = nullptr;
	if (Mesh == nullptr || Mesh->GetRenderData() == nullptr || !Mesh->GetRenderData()->LODResources.IsValidIndex(LODIndex))
	{
		return;
	}
	if (!Mesh->bAllowCPUAccess)
	{

	}
	Data.Clear();

	while (true)
	{
		const FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[LODIndex];
		if (!LOD.Sections.IsValidIndex(SectionIndex))
		{
			Data.CountSections(); return;
		}
		TMap<int32, int32> MeshToSectionVertMap = {};
		uint32 i = 0, is = LOD.Sections[SectionIndex].FirstIndex, l = LOD.Sections[SectionIndex].FirstIndex + LOD.Sections[SectionIndex].NumTriangles * 3;
		FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();
		uint32 il = Indices.Num();
		const bool hasColors = LOD.VertexBuffers.ColorVertexBuffer.GetNumVertices() >= LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		for (i = is; i < l; ++i) {
			if (i < il)
			{
				vi = Indices[i];
				NewIndexPtr = MeshToSectionVertMap.Find(vi);
				if (NewIndexPtr != nullptr)
				{
					svi = *NewIndexPtr;
				}
				else
				{
					Data.Verts.Emplace(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(vi));
					Data.Normals.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(vi));
					Data.UVs.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(vi, 0));
					Data.Sects.Emplace(sec);
					if (hasColors)
					{
						Data.Colors.Emplace(LOD.VertexBuffers.ColorVertexBuffer.VertexColor(vi));
					}
					svi = n;
					MeshToSectionVertMap.Emplace(vi, n);
					++n;
				}
				Data.Tris.Emplace(svi);
			}

		}

		if (!GetAllSections)
		{
			Data.CountSections(); return;
		}
		SectionIndex += 1;
		sec += 1;
		Data.NumSections += 1;
	}
}
void ABulletHoleWall::SetColorData(FMeshData& Data, FLinearColor Color)
{
	Data.Colors = {};
	Data.Colors.SetNumUninitialized(Data.Verts.Num());
	for (int x = 0; x < Data.Verts.Num(); ++x)
	{
		Data.Colors[x] = Color;
	}
}
