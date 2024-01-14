// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/PropBase.h"
#include "ProceduralMeshComponent.h"

// Sets default values
APropBase::APropBase()
{
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	SetRootComponent(ProceduralMesh);

	//메쉬 데이터에 스테틱 메쉬 데이터 넣기
	GetMeshDataFromStaticMesh(Mesh1, &Data1, 0,0,false);
	UnifyTri(Data1);
	GetMeshDataFromStaticMesh(Mesh2, &Data2, 0, 0, false);
	UnifyTri(Data2);

}

// Called when the game starts or when spawned
void APropBase::BeginPlay()
{
	Super::BeginPlay();
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector2D> UVs;

	Vertices.Add(FVector(-50, 0, 50));
	Vertices.Add(FVector(-50, 0, -50));
	Vertices.Add(FVector(50, 0, 50));
	Vertices.Add(FVector(50, 0, -50));

	UVs.Add(FVector2D(0, 0));
	UVs.Add(FVector2D(0, 1));
	UVs.Add(FVector2D(1, 0));
	UVs.Add(FVector2D(1, 1));

	//Triangle1
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);

	//Triangle2
	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(3);
	ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, TArray<FVector>(), UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void APropBase::GetMeshDataFromStaticMesh(UStaticMesh* Mesh, FMeshData* Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections)
{
	int32 n = 0, svi = 0, vi = 0, sec = 0;
	int32* NewIndexPtr = nullptr;

	if (!Mesh) return;

	if(!Mesh->bAllowCPUAccess) 
	{
		UE_LOG(LogTemp, Log, TEXT("Not Allow CpuAccesss"));
	}
	Data->Clear();

	while (true)
	{
		const FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[LODIndex];

		if (!LOD.Sections.IsValidIndex(SectionIndex))
		{
			Data->CountSections();
			return;
		}
		TMap<int32, int32> MeshToSectionVertMap = {};
		uint32 i = 0;
		uint32 is = LOD.Sections[SectionIndex].FirstIndex;
		uint32 l = LOD.Sections[SectionIndex].FirstIndex + LOD.Sections[SectionIndex].NumTriangles * 3;
		FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();
		uint32 il = Indices.Num();
		const bool hasColors = LOD.VertexBuffers.ColorVertexBuffer.GetNumVertices() >= LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();

		for (i = is; i < l; ++i) {
			if (i < il) {
				vi = Indices[i];
				NewIndexPtr = MeshToSectionVertMap.Find(vi);
				if (NewIndexPtr != nullptr) { svi = *NewIndexPtr; }
				else {
					Data->Verts.Emplace(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(vi));
					Data->Verts.Emplace(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(vi));
					Data->Normals.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(vi));
					Data->UVs.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(vi, 0));
					Data->Sects.Emplace(sec);
					if (hasColors) { Data->Colors.Emplace(LOD.VertexBuffers.ColorVertexBuffer.VertexColor(vi)); }
					svi = n;
					MeshToSectionVertMap.Emplace(vi, n);
					++n;
				}
				Data->Tris.Emplace(svi);
			}

		}
	}
}

void APropBase::UnifyTri(FMeshData& Data)
{
	SplitVertexes(Data);
	const TArray<FVector> oldverts = Data.Verts;
	const TArray<FVector> oldnorm = Data.Normals;
	const TArray<FVector2D> olduvs = Data.UVs;
	const TArray<FLinearColor> oldcolors = Data.Colors;
	const TArray<int> oldtris = Data.Tris;
	int vl = Data.Verts.Num();
	bool hasNormals = (Data.Normals.Num() >= vl), hasUVs = (Data.UVs.Num() >= vl), hasColors = (Data.Colors.Num() >= vl);
	Data.Verts = {};
	Data.Tris = {};
	Data.Normals = {};
	Data.UVs = {};
	Data.Colors = {};
	int x = 0, l = oldtris.Num();
	for (x = 0; x < l; ++x) {
		Data.Verts.Emplace(oldverts[oldtris[x]]);
		if (hasNormals) { Data.Normals.Emplace(oldnorm[oldtris[x]]); }
		if (hasUVs) { Data.UVs.Emplace(olduvs[oldtris[x]]); }
		if (hasColors) { Data.Colors.Emplace(oldcolors[oldtris[x]]); }
		Data.Tris.Emplace(x);
	}
}

void APropBase::SplitVertexes(FMeshData& Data)
{
	TMap<int, int> Visited = {};
	int vl = Data.Verts.Num();
	FVector vec; FVector2D uv; FLinearColor col; int sect; //can't Emplace from same array so have to make copies
	const bool hasNormals = Data.Normals.Num() >= vl, hasUVs = Data.UVs.Num() >= vl, hasColors = Data.Colors.Num() >= vl, hasSects = Data.Sects.Num() >= vl;
	int x = 0, l = Data.Tris.Num();
	for (x = 0; x < l; ++x) {
		if (!Visited.Contains(Data.Tris[x])) {
			Visited.Emplace(Data.Tris[x], 1);
		}
		else {
			vec = Data.Verts[Data.Tris[x]]; Data.Verts.Emplace(vec);
			if (hasNormals) { vec = Data.Normals[Data.Tris[x]]; Data.Normals.Emplace(vec); }
			if (hasUVs) { uv = Data.UVs[Data.Tris[x]]; Data.UVs.Emplace(uv); }
			if (hasColors) { col = Data.Colors[Data.Tris[x]]; Data.Colors.Emplace(col); }
			if (hasSects) { sect = Data.Sects[Data.Tris[x]]; Data.Sects.Emplace(sect); }
			Data.Tris[x] = vl;
			++vl;
		}
	}
}



// Called every frame
void APropBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

