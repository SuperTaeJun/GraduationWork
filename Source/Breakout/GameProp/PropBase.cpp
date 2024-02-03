

#include "GameProp/PropBase.h"
#include "ProceduralMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Character/CharacterBase.h"

//전역변수
TMap<FMeshData*, TArray<FMeshData>> CachedSections = {};
TMap<FName, FMeshData> MeshDatas = {};

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
	ConstructorHelpers::FObjectFinder<UStaticMesh> SMMesh1(TEXT("/Engine/BasicShapes/Cone.Cone"));
	////메쉬 데이터에 스테틱 메쉬 데이터 넣기
	GetMeshDataFromStaticMesh(SMMesh1.Object, Data1, 0, 0, true);
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

	if (AreaSphere)
	{
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &APropBase::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &APropBase::OnSphereEndOverlap);
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
			if (i < il) {
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

//void APropBase::UnifyTri(FMeshData& Data)
//{
//	SplitVertexes(Data);
//	const TArray<FVector> oldverts = Data.Verts;
//	const TArray<FVector> oldnorm = Data.Normals;
//	const TArray<FVector2D> olduvs = Data.UVs;
//	const TArray<FLinearColor> oldcolors = Data.Colors;
//	const TArray<int> oldtris = Data.Tris;
//	int vl = Data.Verts.Num();
//	bool hasNormals = (Data.Normals.Num() >= vl), hasUVs = (Data.UVs.Num() >= vl), hasColors = (Data.Colors.Num() >= vl);
//	Data.Verts = {};
//	Data.Tris = {};
//	Data.Normals = {};
//	Data.UVs = {};
//	Data.Colors = {};
//	int x = 0, l = oldtris.Num();
//	for (x = 0; x < l; ++x) {
//		Data.Verts.Emplace(oldverts[oldtris[x]]);
//		if (hasNormals) { Data.Normals.Emplace(oldnorm[oldtris[x]]); }
//		if (hasUVs) { Data.UVs.Emplace(olduvs[oldtris[x]]); }
//		if (hasColors) { Data.Colors.Emplace(oldcolors[oldtris[x]]); }
//		Data.Tris.Emplace(x);
//	}
//}
//
//void APropBase::SplitVertexes(FMeshData& Data)
//{
//	TMap<int, int> Visited = {};
//	int vl = Data.Verts.Num();
//	FVector vec; FVector2D uv; FLinearColor col; int sect; 
//	const bool hasNormals = Data.Normals.Num() >= vl, hasUVs = Data.UVs.Num() >= vl, hasColors = Data.Colors.Num() >= vl, hasSects = Data.Sects.Num() >= vl;
//	int x = 0, l = Data.Tris.Num();
//	for (x = 0; x < l; ++x) {
//		if (!Visited.Contains(Data.Tris[x])) {
//			Visited.Emplace(Data.Tris[x], 1);
//		}
//		else {
//			vec = Data.Verts[Data.Tris[x]]; Data.Verts.Emplace(vec);
//			if (hasNormals) { vec = Data.Normals[Data.Tris[x]]; Data.Normals.Emplace(vec); }
//			if (hasUVs) { uv = Data.UVs[Data.Tris[x]]; Data.UVs.Emplace(uv); }
//			if (hasColors) { col = Data.Colors[Data.Tris[x]]; Data.Colors.Emplace(col); }
//			if (hasSects) { sect = Data.Sects[Data.Tris[x]]; Data.Sects.Emplace(sect); }
//			Data.Tris[x] = vl;
//			++vl;
//		}
//	}
//}
//
//void APropBase::TransformMeshData(FMeshData& Data, FTransform Transform, FVector Pivot)
//{
//	const FVector ZeroVector = FVector(0.0f, 0.0f, 0.0f);
//	FMeshData newdata;
//	FMeshData* pres = &newdata;
//	if (InPlace) { pres = &Data; }
//	else { newdata = Data; }
//	FMeshData& res = *pres;
//	const FVector loc = Transform.GetLocation();
//	const FRotator rot = Transform.Rotator();
//	const FVector scale = Transform.GetScale3D();
//	bool skiprot = (rot == FRotator(0.0f, 0.0f, 0.0f));
//	bool skipscale = (scale == FVector(1.0f, 1.0f, 1.0f));
//	bool skippiv = (Pivot == ZeroVector);
//	int x = 0, l = res.Verts.Num(), nl = res.Normals.Num();
//	bool hasNormals = (nl >= l);
//	for (x = 0; x < l; ++x) {
//		FVector& v = res.Verts[x];
//		if (skippiv) { if (skipscale) { if (skiprot) { v += loc; } else { v = rot.RotateVector(v) + loc; } } else { v = rot.RotateVector(v * scale) + loc; } }
//		else { if (skipscale) { v = rot.RotateVector((v - Pivot)) + Pivot + loc; } else { v = rot.RotateVector(((v - Pivot) * scale)) + Pivot + loc; } }
//		if (hasNormals) { if (!skiprot) { FVector& n = res.Normals[x]; n = rot.RotateVector(n); } }
//	}
//}

void APropBase::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//ACharacterBase* CharacterBase = Cast<ACharacterBase>(OtherActor);

	//CharacterBase->SetbCanObtainEscapeTool(true);
	//CharacterBase->OverlappingEscapeTool = this;

	////UE_LOG(LogTemp, Log, TEXT("OBTAIN"));
}

void APropBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//if (Mesh->bHiddenInGame==false)
	//{
	//	ACharacterBase* CharacterBase = Cast<ACharacterBase>(OtherActor);

	//	CharacterBase->SetbCanObtainEscapeTool(false);
	//	CharacterBase->OverlappingEscapeTool = nullptr;

	//}
}

void APropBase::AddTriangle(int32 V1, int32 V2, int32 V3)
{
	Triangles.Add(V1);
	Triangles.Add(V2);
	Triangles.Add(V3);
}

void APropBase::ProceduralMeshFromMeshData(UProceduralMeshComponent* Mesh, FMeshData& Data, int SectionIndex, bool Collision, bool CalcTangents)
{
	if (!Mesh) return;

	TArray<FMeshData> ConvertOutput = ConvertFromSectionedMeshData(Data);
	CachedSections.Emplace(&Data, ConvertOutput);
	TArray<FProcMeshTangent> Tangents = {};
	for (int x = 0; x < ConvertOutput.Num(); ++x)
	{
		FMeshData& r = ConvertOutput[x];
		Mesh->CreateMeshSection_LinearColor(SectionIndex + x, r.Verts, r.Tris, r.Normals, r.UVs, r.Colors, Tangents, Collision);
		return;
	}
	Mesh->CreateMeshSection_LinearColor(SectionIndex, Data.Verts, Data.Tris, Data.Normals, Data.UVs, Data.Colors, Tangents, Collision);
}

TArray<FMeshData> APropBase::ConvertFromSectionedMeshData(FMeshData& Data)
{


	return TArray<FMeshData>();
}



// Called every frame
void APropBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (Mesh->bHiddenInGame == true)
	//	Destroy();
}

void APropBase::SetHideMesh()
{
	//Mesh->bHiddenInGame = true;
}

