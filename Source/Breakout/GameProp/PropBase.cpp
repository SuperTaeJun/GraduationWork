

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


	////메쉬 데이터에 스테틱 메쉬 데이터 넣기
	if (Mesh1)
	{
		UE_LOG(LogTemp, Log, TEXT("HAHAHAHA"));
		GetMeshDataFromStaticMesh(Mesh1, &Data1, 0, 0, true);
		//ProceduralMeshFromMeshData(ProceduralMesh, Data1, 0, false, false);
		TArray<FProcMeshTangent> Tangents = {};
		ProceduralMesh->CreateMeshSection_LinearColor(0, Data1.Verts, Data1.Tris, Data1.Normals, Data1.UVs, Data1.Colors, Tangents, false);
	}

	Vertices.Add(FVector(0, -100, 0)); //lower left - 0
	Vertices.Add(FVector(0, -100, 100)); //upper left - 1
	Vertices.Add(FVector(0, 100, 0)); //lower right - 2 
	Vertices.Add(FVector(0, 100, 100)); //upper right - 3

	Vertices.Add(FVector(100, -100, 0)); //lower front left - 4
	Vertices.Add(FVector(100, -100, 100)); //upper front left - 5

	Vertices.Add(FVector(100, 100, 100)); //upper front right - 6
	Vertices.Add(FVector(100, 100, 0)); //lower front right - 7

	//Back face of cube
	AddTriangle(0, 2, 3); //normal face
	//AddTriangle(3, 2, 0); // flipped face
	AddTriangle(3, 1, 0);

	//Left face of cube
	AddTriangle(0, 1, 4);
	AddTriangle(4, 1, 5);

	//Front face of cube
	AddTriangle(4, 5, 7);
	AddTriangle(7, 5, 6);

	//Right face of cube
	AddTriangle(7, 6, 3);
	AddTriangle(3, 2, 7);

	//Top face
	AddTriangle(1, 3, 5);
	AddTriangle(6, 5, 3);

	//bottom face
	AddTriangle(2, 0, 4);
	AddTriangle(4, 7, 2);

	TArray<FLinearColor> VertexColors;
	VertexColors.Add(FLinearColor(0.f, 0.f, 1.f));
	VertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
	VertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
	VertexColors.Add(FLinearColor(0.f, 1.f, 0.f));
	VertexColors.Add(FLinearColor(0.5f, 1.f, 0.5f));
	VertexColors.Add(FLinearColor(0.f, 1.f, 0.f));
	VertexColors.Add(FLinearColor(1.f, 1.f, 0.f));
	VertexColors.Add(FLinearColor(0.f, 1.f, 1.f));

	ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), TArray<FVector2D>(), VertexColors, TArray<FProcMeshTangent>(), true);
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
				else 
				{
					Data->Verts.Emplace(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(vi));
					Data->Verts.Emplace(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(vi));
					Data->Normals.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(vi));
					Data->UVs.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(vi, 0));
					Data->Sects.Emplace(sec);
					if (hasColors) 
					{ 
						Data->Colors.Emplace(LOD.VertexBuffers.ColorVertexBuffer.VertexColor(vi)); 
					}
					svi = n;
					MeshToSectionVertMap.Emplace(vi, n);
					++n;
				}
				Data->Tris.Emplace(svi);
			}

		}
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

