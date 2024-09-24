// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/BulletHoleWall.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
#include "Operations/MeshBoolean.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/ProjectileBase.h"
//using namespace UE::Geometry;

ABulletHoleWall::ABulletHoleWall()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultRoot = CreateDefaultSubobject<USceneComponent>("DefaultRootComponent");
	RootComponent = DefaultRoot;
	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProceduralMesh->SetupAttachment(RootComponent);
	Sphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(RootComponent);

	Hp = 50.f;
	bDestroyed = false;
}

void ABulletHoleWall::BeginPlay()
{
	Super::BeginPlay();
	OnTakeAnyDamage.AddDynamic(this, &ABulletHoleWall::ReciveDamage);
	Hp = 50.f;
	bDestroyed = false;
	ResetMeshData = MeshDataA;
	TArray<FProcMeshTangent> Tangents = {};
	ProceduralMesh->CreateMeshSection_LinearColor(0, MeshDataA.Verts, MeshDataA.Tris, MeshDataA.Normals, MeshDataA.UVs, MeshDataA.Colors, Tangents, true);
}
void ABulletHoleWall::ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Hp -= Damage;

	if (Hp <= 0.f && !bDestroyed)
	{
		//저장 부셔진 조각들 계산해서 여기는 수정해야함
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				for (int k = 0; k < 1; ++k)
				{
					FVector Loc = FVector(i, j, k) * 40.f;
					FTransform DataATransform;
					FTransform SculptureTransform;
					SculptureTransform.SetLocation(Loc);
					SculptureTransform.SetScale3D(FVector(0.5, 0.5, 0.5));
					MeshDataStorage.Add(MeshBoolean(MeshDataA, DataATransform, SetRandomVertex(SculptureData, -3.f, 10.f, 0.1f), SculptureTransform, false));

				}
			}
		}
		MeshSculptures.SetNum(MeshDataStorage.Num());
		//각각 조각들 설정
		for (int i = 0; i < MeshDataStorage.Num(); ++i)
		{
			FTransform AddTransform;
			MeshSculptures[i] = Cast<UProceduralMeshComponent>(AddComponentByClass(UProceduralMeshComponent::StaticClass(), false, AddTransform, false));
			AddInstanceComponent(MeshSculptures[i]);
			if (MeshSculptures[i])
			{
				TArray<FProcMeshTangent> Tangents = {};

				MeshSculptures[i]->bUseComplexAsSimpleCollision = false;
				MeshSculptures[i]->SetSimulatePhysics(true);
				MeshSculptures[i]->AddCollisionConvexMesh(MeshDataStorage[i].Verts);
				MeshSculptures[i]->CreateMeshSection_LinearColor(0, MeshDataStorage[i].Verts, MeshDataStorage[i].Tris, MeshDataStorage[i].Normals, MeshDataStorage[i].UVs, MeshDataStorage[i].Colors, Tangents, true);
				//ProceduralMesh->DestroyComponent();
				if (i == 0)
				{
					ProceduralMesh->SetHiddenInGame(true);
					ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				};
			}
		}
		GetWorldTimerManager().SetTimer
		(
			DestroyTimer,
			this,
			&ABulletHoleWall::AllDestroy,
			4.f
		);
		bDissolve = true;
		Hp = 999999;
	}
}
void ABulletHoleWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDissolve)
	{
		for (int i = 0; i < MeshSculptures.Num(); ++i)
		{
			MDynamicDissolveInst = UMaterialInstanceDynamic::Create(MDissolveInst, this);
			MeshSculptures[i]->SetMaterial(0, MDynamicDissolveInst);
			DissolvePercent += DeltaTime / 10;
			MDynamicDissolveInst->SetScalarParameterValue(FName("Dissolve"), DissolvePercent);
		}
	}
}

void ABulletHoleWall::SetBulletHole(const FVector SweepResult)
{
	HitLoc = SweepResult;
	Sphere->SetWorldLocation(HitLoc);


	FTransform ATransform = ProceduralMesh->GetRelativeTransform();
	FTransform BTransform;
	BTransform.SetLocation(Sphere->GetRelativeTransform().GetLocation());
	BTransform.SetRotation(ProceduralMesh->GetRelativeRotation().Quaternion());
	BTransform.SetScale3D(FVector(60.f, 0.2f, 0.2f));

	MeshDataA = MeshBoolean(MeshDataA, ATransform, SetRandomVertex(MeshDataB, -20.f, 20.f, 0.001), BTransform, true);

	TArray<FProcMeshTangent> Tangents = {};
	ProceduralMesh->CreateMeshSection_LinearColor(0, MeshDataA.Verts, MeshDataA.Tris, MeshDataA.Normals, MeshDataA.UVs, MeshDataA.Colors, Tangents, true);
}

FMeshData ABulletHoleWall::MeshBoolean(UPARAM(ref)FMeshData DataA, FTransform TransformA, UPARAM(ref)FMeshData DataB, FTransform TransformB, bool OptionType)
{
	UE::Geometry::FDynamicMesh3 BooleanOutput;
	BooleanOutput.EnableAttributes();
	BooleanOutput.EnableVertexColors(FVector3f(0.0f, 0.0f, 0.0f));
	BooleanOutput.EnableVertexNormals(FVector3f(0.0f, 0.0f, 0.0f));
	BooleanOutput.EnableVertexUVs(FVector2f(0.0f, 0.0f));

	UE::Geometry::FMeshBoolean::EBooleanOp Option;
	if (OptionType)
		Option = UE::Geometry::FMeshBoolean::EBooleanOp::Difference;
	else
		Option = UE::Geometry::FMeshBoolean::EBooleanOp::Intersect;

	FTransform3d ConvertedTransformA = ConvertToFTransform3d(TransformA);
	FTransform3d ConvertedTransformB = ConvertToFTransform3d(TransformB);
	UE::Geometry::FDynamicMesh3 DMeshA = ConvertToFDynamicMesh3(DataA);
	UE::Geometry::FDynamicMesh3 DMeshB = ConvertToFDynamicMesh3(DataB);

	//MESH BOOLEAN 결과를 BooleanOutput에 저장
	UE::Geometry::FMeshBoolean Boolean(&DMeshA, ConvertedTransformA,
		&DMeshB, ConvertedTransformB,
		&BooleanOutput, Option);

	Boolean.bCollapseDegenerateEdgesOnCut = true;
	Boolean.bPreserveOverlayUVs = true;
	Boolean.bPreserveVertexUVs = true;
	Boolean.bPutResultInInputSpace = true;

	Boolean.bSimplifyAlongNewEdges = false;
	Boolean.SimplificationAngleTolerance = 0.f;
	Boolean.bTrackAllNewEdges = false;
	Boolean.bWeldSharedEdges = false;

	Boolean.DegenerateEdgeTolFactor = 0.1f;
	Boolean.WindingThreshold = 0.1f;
	Boolean.SnapTolerance = 0.0000001f;

	Boolean.Compute();

	return 	ConverToFMeshData(BooleanOutput, DataA);
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
	for (int x = 0; x < Data.Tris.Num(); x += 3)
	{
		if (x + 2 < Data.Tris.Num())
		{
			ResultTri.A = Data.Tris[x];
			ResultTri.B = Data.Tris[x + 1];
			ResultTri.C = Data.Tris[x + 2];

			Result.AppendTriangle(ResultTri);
		}
	}
	return Result;
}

FMeshData ABulletHoleWall::ConverToFMeshData(UE::Geometry::FDynamicMesh3& Input, FMeshData& Output)
{
	FMeshData Result = Output;
	UE::Geometry::FDynamicMesh3* Data = &Input;

	int TriNum = Data->TriangleCount();
	int VertexNum = Data->TriangleCount() * 3;

	Result.Verts.SetNumUninitialized(VertexNum);
	Result.Normals.SetNumUninitialized(VertexNum);
	Result.UVs.SetNumUninitialized(VertexNum);
	Result.Colors.SetNumUninitialized(VertexNum);
	Result.Sects.SetNumUninitialized(VertexNum);

	Result.Tris.SetNumUninitialized(TriNum * 3);

	FVector3d vertex1, vertex2, vertex3;
	int x = 0;
	int y = 0;
	int z = 0;
	for (auto TriID : Data->TriangleIndicesItr())
	{
		y = x + 1;
		z = y + 1;

		UE::Geometry::FIndex3i TriVerts = Data->GetTriangle(TriID);
		Data->GetTriVertices(TriID, vertex1, vertex2, vertex3);

		//Vertex추가
		Result.Verts[x].X = vertex1.X; 		Result.Verts[x].Y = vertex1.Y; 		Result.Verts[x].Z = vertex1.Z;
		Result.Verts[y].X = vertex2.X; 		Result.Verts[y].Y = vertex2.Y; 		Result.Verts[y].Z = vertex2.Z;
		Result.Verts[z].X = vertex3.X; 		Result.Verts[z].Y = vertex3.Y; 		Result.Verts[z].Z = vertex3.Z;

		Result.Sects[x] = 0; 	Result.Sects[y] = 0; 	Result.Sects[z] = 0;

		//노멀추가
		if (Data->HasVertexNormals())
		{
			Result.Normals[x] = (FVector)Data->GetVertexNormal(TriVerts.A);
			Result.Normals[y] = (FVector)Data->GetVertexNormal(TriVerts.B);
			Result.Normals[z] = (FVector)Data->GetVertexNormal(TriVerts.C);
		}
		else
		{
			Result.Normals[x] = FVector(0.0f, 0.0f, 1.0f);
			Result.Normals[y] = FVector(0.0f, 0.0f, 1.0f);
			Result.Normals[z] = FVector(0.0f, 0.0f, 1.0f);
		}

		//UV추가
		if (Data->HasVertexUVs())
		{
			Result.UVs[x] = (FVector2D)Data->GetVertexUV(TriVerts.A);
			Result.UVs[y] = (FVector2D)Data->GetVertexUV(TriVerts.B);
			Result.UVs[z] = (FVector2D)Data->GetVertexUV(TriVerts.C);
		}
		else
		{
			Result.UVs[x] = FVector2D(0.0f, 0.0f);
			Result.UVs[y] = FVector2D(0.0f, 0.0f);
			Result.UVs[z] = FVector2D(0.0f, 0.0f);
		}

		//Color추가
		if (Data->HasVertexColors())
		{
			Result.Colors[x] = (FLinearColor)Data->GetVertexColor(TriVerts.A);
			Result.Colors[y] = (FLinearColor)Data->GetVertexColor(TriVerts.B);
			Result.Colors[z] = (FLinearColor)Data->GetVertexColor(TriVerts.C);
		}
		else
		{
			Result.Colors[x] = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
			Result.Colors[y] = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
			Result.Colors[z] = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
		}

		Result.Tris[x] = x;
		Result.Tris[y] = y;
		Result.Tris[z] = z;
		x += 3;
	}

	return Result;
}

FTransform3d ABulletHoleWall::ConvertToFTransform3d(FTransform Input)
{
	const FVector& Location = Input.GetLocation();
	const FVector& Scale = Input.GetScale3D();

	return FTransform3d(FQuat(Input.GetRotation()),
		FVector3d(Location.X, Location.Y, Location.Z),
		FVector3d(Scale.X, Scale.Y, Scale.Z)
	);
}

FMeshData ABulletHoleWall::TransformMeshData(UPARAM(ref) FMeshData& Data, FTransform Transform, bool InPlace, FVector Pivot)
{
	FMeshData newdata;
	FMeshData* pres = &newdata;
	if (InPlace) { pres = &Data; }
	else { newdata = Data; }
	FMeshData& res = *pres;
	const FVector loc = Transform.GetLocation();
	const FRotator rot = Transform.Rotator();
	const FVector scale = Transform.GetScale3D();
	bool skiprot = (rot == FRotator(0.0f, 0.0f, 0.0f));
	bool skipscale = (scale == FVector(1.0f, 1.0f, 1.0f));
	bool skippiv = (Pivot == FVector(0.0f, 0.0f, 0.0f));
	int x = 0;
	int l = res.Verts.Num();
	int nl = res.Normals.Num();
	bool hasNormals = (nl >= l);
	for (x = 0; x < l; ++x) {
		FVector& v = res.Verts[x];
		if (skippiv)
		{
			if (skipscale)
			{
				if (skiprot)
				{
					v += loc;
				}
				else
				{
					v = rot.RotateVector(v) + loc;
				}
			}
			else
			{
				v = rot.RotateVector(v * scale) + loc;
			}
		}
		else
		{
			if (skipscale)
			{
				v = rot.RotateVector((v - Pivot)) + Pivot + loc;
			}
			else
			{
				v = rot.RotateVector(((v - Pivot) * scale)) + Pivot + loc;
			}
		}
		if (hasNormals)
		{
			if (!skiprot) {
				FVector& n = res.Normals[x]; n = rot.RotateVector(n);
			}
		}
	}
	return newdata;
}

void ABulletHoleWall::AllDestroy()
{
	for (int i = 0; i < MeshDataStorage.Num(); ++i)
	{
		if (MeshSculptures[i])
		{
			MeshSculptures[i]->DestroyComponent();
			MeshSculptures[i] = nullptr;
		}
	}
	MeshDataStorage.Empty();
	bDissolve = false;
	DissolvePercent = -3.f;
	Hp = 50.f;
	SetActorLocation(FVector(0.f, 0.f, -1000.f));
	ProceduralMesh->SetHiddenInGame(false);
	ProceduralMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshDataA = ResetMeshData;
	TArray<FProcMeshTangent> Tangents = {};
	ProceduralMesh->CreateMeshSection_LinearColor(0, MeshDataA.Verts, MeshDataA.Tris, MeshDataA.Normals, MeshDataA.UVs, MeshDataA.Colors, Tangents, true);
	bUsing = true;
}
void ABulletHoleWall::GetMeshDataFromStaticMesh(UStaticMesh* Mesh, UPARAM(ref) FMeshData& Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections)
{
	int32 VertexCount = 0, SectionVertexIndex = 0, VertexIndex = 0, SectionID = 0;

	int32* NewIndexPtr = nullptr;
	if (Mesh == nullptr || Mesh->GetRenderData() == nullptr || !Mesh->GetRenderData()->LODResources.IsValidIndex(LODIndex))
	{
		return;
	}
	Data.Clear();

	while (true)
	{
		// 현재 LOD에 대한 리소스를 가져옴
		const FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[LODIndex];

		if (!LOD.Sections.IsValidIndex(SectionIndex))
		{
			return;
		}

		// 버텍스 재사용을 위한 맵 생성
		TMap<int32, int32> MeshToSectionVertMap = {};
		uint32 TriangleIndex = 0;
		uint32	FirstIndex = LOD.Sections[SectionIndex].FirstIndex;
		uint32	LastIndex = FirstIndex + LOD.Sections[SectionIndex].NumTriangles * 3;
		
		FIndexArrayView Indices = LOD.IndexBuffer.GetArrayView();
		uint32 il = Indices.Num();
		const bool hasColors = LOD.VertexBuffers.ColorVertexBuffer.GetNumVertices() >= LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		for (TriangleIndex = FirstIndex; TriangleIndex < LastIndex; ++TriangleIndex)
		{
			if (TriangleIndex < il)
			{
				VertexIndex = Indices[TriangleIndex];
				NewIndexPtr = MeshToSectionVertMap.Find(VertexIndex);
				if (NewIndexPtr != nullptr)
				{
					// 이미 매핑된 버텍스 인덱스 사용
					SectionVertexIndex = *NewIndexPtr;
				}
				else
				{
					// 새로운 버텍스 데이터를 수집
					Data.Verts.Emplace(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex));
					Data.Normals.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex));
					Data.UVs.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, 0));
					Data.Sects.Emplace(SectionID);
					if (hasColors)
					{
						Data.Colors.Emplace(LOD.VertexBuffers.ColorVertexBuffer.VertexColor(VertexIndex));
					}

					// 새 버텍스 매핑 추가
					SectionVertexIndex = VertexCount;
					MeshToSectionVertMap.Emplace(VertexIndex, VertexCount);
					++VertexCount;
				}
				Data.Tris.Emplace(SectionVertexIndex);
			}

		}

		if (!GetAllSections)
		{
			return;
		}
		SectionIndex += 1;
		SectionID += 1;
		Data.NumSections += 1;
	}
}

void ABulletHoleWall::SetColorData(UPARAM(ref) FMeshData& Data, FLinearColor Color)
{
	Data.Colors = {};
	Data.Colors.SetNumUninitialized(Data.Verts.Num());
	for (int x = 0; x < Data.Verts.Num(); ++x)
	{
		Data.Colors[x] = Color;
	}
}

FMeshData ABulletHoleWall::SetRandomVertex(UPARAM(ref)FMeshData& MeshData, float Min, float Max, float Tolerance)
{

	FMeshData Result = MeshData;
	TMap<FVector, FVector> Already = {};
	Tolerance = 1.0f / Tolerance;
	FVector tCoord;

	for (int x = 0; x < MeshData.Verts.Num(); ++x)
	{
		tCoord = FVector(Result.Verts[x].X * Tolerance, Result.Verts[x].Y * Tolerance, Result.Verts[x].Z * Tolerance);

		//이미 했던건지 확인
		if (Already.Contains(tCoord))
		{
			Result.Verts[x] = Already[tCoord];
		}
		else
		{
			if (MeshData.Normals.IsValidIndex(x))
			{
				Result.Verts[x] = MeshData.Verts[x] + MeshData.Normals[x] * FMath::RandRange(Min, Max) + FMath::VRand() * FMath::RandRange(Min, Max);
			}
			else
			{
				Result.Verts[x] = MeshData.Verts[x] + FMath::VRand() * FMath::RandRange(Min, Max);
			}
			//안했던거는 맵에 추가
			Already.Emplace(tCoord, Result.Verts[x]);
		}
	}
	return Result;
}
