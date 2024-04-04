

#include "GameProp/PropBase.h"
#include "ProceduralMeshComponent.h"
#include "Components/SphereComponent.h"

APropBase::APropBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	SetRootComponent(AreaSphere);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProceduralMesh->SetupAttachment(RootComponent);

}

void APropBase::BeginPlay()
{
	Super::BeginPlay();

}

void APropBase::UnifyTri(UPARAM(ref) FMeshData& Data)
{
	//UE_LOG(LogTemp, Log, TEXT("%d"), Data1.Tris.Num());

	TMap<int, int> Visited = {};
	int vl = Data.Verts.Num();
	FVector vec; 
	FVector2D uv; 
	FLinearColor col; 
	int sect; 
	bool hasNormals = Data.Normals.Num() >= vl;
	bool	hasUVs = Data.UVs.Num() >= vl;
	bool	hasColors = Data.Colors.Num() >= vl;
	bool	hasSects = Data.Sects.Num() >= vl;
	int x = 0, l = Data.Tris.Num();

	for (x = 0; x < l; ++x) 
	{
		if (!Visited.Contains(Data.Tris[x])) 
		{
			Visited.Emplace(Data.Tris[x], 1);

		}
		else 
		{
			vec = Data.Verts[Data.Tris[x]]; 
			Data.Verts.Emplace(vec);
			if (hasNormals) 
				 vec = Data.Normals[Data.Tris[x]]; 
			Data.Normals.Emplace(vec); 

			if (hasUVs)
				 uv = Data.UVs[Data.Tris[x]]; 
			Data.UVs.Emplace(uv); 

			if (hasColors) 
				 col = Data.Colors[Data.Tris[x]]; 
			Data.Colors.Emplace(col); 

			if (hasSects)
				 sect = Data.Sects[Data.Tris[x]]; 
			Data.Sects.Emplace(sect); 

			Data.Tris[x] = vl;
			++vl;

		}
		UE_LOG(LogTemp, Log, TEXT("%s"), *Data.Verts[Data.Tris[0]].ToString());

	}

	//기존 정보를 저장하고 다시 트라이앵글 기준으로 정렬
	TArray<FVector> oldverts = Data.Verts;
	TArray<FVector> oldnorm = Data.Normals;
	TArray<FVector2D> olduvs = Data.UVs;
	TArray<FLinearColor> oldcolors = Data.Colors;
	TArray<int32> oldtris = Data.Tris;
	UE_LOG(LogTemp, Warning, TEXT("TRI = %d"), Data.Tris[0]);
	UE_LOG(LogTemp, Warning, TEXT("TRI = %d"), Data.Tris[50]);
	vl = Data.Verts.Num();
	hasNormals = (Data.Normals.Num() >= vl);
	hasUVs = (Data.UVs.Num() >= vl);
	hasColors = (Data.Colors.Num() >= vl);
	Data.Verts = {};
	Data.Tris = {};
	Data.Normals = {};
	Data.UVs = {};
	Data.Colors = {};
	x = 0;
	l = oldtris.Num();
	for (x = 0; x < l; ++x) 
	{
		Data.Verts.Emplace(oldverts[oldtris[x]]);
		if (hasNormals) 
			Data.Normals.Emplace(oldnorm[oldtris[x]]);
		if (hasUVs) 
			 Data.UVs.Emplace(olduvs[oldtris[x]]); 
		if (hasColors)
			 Data.Colors.Emplace(oldcolors[oldtris[x]]); 
		Data.Tris.Emplace(x);
	}

}

void APropBase::InterpMeshData(FMeshData& Data, FMeshData& DataA, FMeshData& DataB, float Alpha, bool Clamp)
{
	int x = 0, l = Data.Verts.Num(), al = DataA.Verts.Num(), bl = DataB.Verts.Num();
	if (l <= 0 || al <= 0 || bl <= 0)
	{
		return; 
	}
	if (Clamp) 
	{
		if (Alpha <= 0.0f) 
		{
			if (Data.Verts[0] != DataA.Verts[0]) { Data = DataA; }
			return;
		}
		if (Alpha >= 1.0f) 
		{
			if (Data.Verts[0] != DataB.Verts[0]) { Data = DataB; }
			return;
		}
	}
	int ml = l = std::min(l, al);
	ml = std::min(ml, bl);
	const bool hasNormals = (Data.Normals.Num() >= ml && DataA.Normals.Num() >= ml && DataB.Normals.Num() >= ml);
	const bool hasUVs = (Data.UVs.Num() >= ml && DataA.UVs.Num() >= ml && DataB.UVs.Num() >= ml);
	const bool hasColors = (Data.Colors.Num() >= ml && DataA.Colors.Num() >= ml && DataB.Colors.Num() >= ml);
	int y = 0;
	for (x = 0; x < l; ++x) 
	{
		y = x;
		if (bl < l && y >= bl) 
		{
			y = (y % bl) / 3; 
		}
		Data.Verts[x] = FMath::Lerp(DataA.Verts[x], DataB.Verts[y], Alpha);
		if (hasNormals) 
		{
			Data.Normals[x] = FMath::Lerp(DataA.Normals[x], DataB.Normals[y], Alpha);
			Data.Normals[x].Normalize();
		}
		if (hasColors) 
		{
			Data.UVs[x] = FMath::Lerp(DataA.UVs[x], DataB.UVs[y], Alpha);
		}
		if (hasColors) 
		{
			Data.Colors[x] = FMath::Lerp(DataA.Colors[x], DataB.Colors[y], Alpha);
		}
	}
}

void APropBase::GetMeshDataFromStaticMesh(UStaticMesh* Mesh, UPARAM(ref) FMeshData& Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections)
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

void APropBase::SetColorData(UPARAM(ref) FMeshData& Data, FLinearColor Color)
{
	Data.Colors = {};
	Data.Colors.SetNumUninitialized(Data.Verts.Num());
	for (int x = 0; x < Data.Verts.Num(); ++x)
	{
		Data.Colors[x] = Color;
	}
}

double APropBase::DegSin(double A)
{
	return FMath::Sin(3.141592/ (180.0) * A);
}

void APropBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

