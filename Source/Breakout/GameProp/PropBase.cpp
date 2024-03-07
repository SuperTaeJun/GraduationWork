

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
	
	//메쉬 에셋 가져오기
	ConstructorHelpers::FObjectFinder<UStaticMesh> SMMesh1(TEXT("/Game/FPS_Weapon_Bundle/Weapons/Meshes/KA74U/SM_KA74U_X.SM_KA74U_X"));

	ConstructorHelpers::FObjectFinder<UStaticMesh> SMMesh2(TEXT("/Game/FPS_Weapon_Bundle/Weapons/Meshes/SMG11/SM_SMG11_X.SM_SMG11_X"));

	////메쉬 데이터에 스테틱 메쉬 데이터 넣기
	GetMeshDataFromStaticMesh(SMMesh1.Object, Data1, 0, 0, true);
	UE_LOG(LogTemp, Log, TEXT("%s"), *Data1.Verts[Data1.Tris[0]].ToString());
	UnifyTri(Data1);
	UE_LOG(LogTemp, Log, TEXT("%s"), *Data1.Verts[Data1.Tris[0]].ToString());
	GetMeshDataFromStaticMesh(SMMesh2.Object, Data2, 0, 0, true);
	UnifyTri(Data2);

	//if (Data1.Verts.Num() < Data2.Verts.Num())
	//{
	//	int Loop = Data2.Verts.Num() - Data1.Verts.Num();

	//	for (int i = 0; i < Loop / 3 - 1; ++i)
	//	{
	//		RandValue = FMath::RandRange(0, Data1.Verts.Num() - 1);

	//		for (int j = 0; j < 2; ++j)
	//		{
	//			Data1.Tris.Add(RandValue);
	//			Data1.Verts.Add(Data1.Verts[RandValue]);
	//			Data1.Normals.Add(FVector(0.f, 0.f, 0.f));
	//			Data1.UVs.Add(FVector2D(0.f, 0.f));
	//			Data1.Colors.Add(FLinearColor::Red);
	//		}

	//	}
	//}

	SetColorData(Data1, FLinearColor::Red);
	SetColorData(Data2, FLinearColor::Red);
	InterpData = Data1;

	TArray<FProcMeshTangent> Tangents = {};
	ProceduralMesh->CreateMeshSection_LinearColor
	(
		0,  //SECK INDEX
		Data1.Verts,  //V
		Data1.Tris, //T
		Data1.Normals, //N
		Data1.UVs, //UV
		Data1.Colors, //C
		Tangents, //T
		false // COLLISION
	);


}

// Called when the game starts or when spawned
void APropBase::BeginPlay()
{
	Super::BeginPlay();

}

void APropBase::UnifyTri(FMeshData& Data)
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
	TArray<int> oldtris = Data.Tris;
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

void APropBase::GetMeshDataFromStaticMesh(UStaticMesh* Mesh, FMeshData& Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections)
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

void APropBase::SetColorData(FMeshData& Data, FLinearColor Color)
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

