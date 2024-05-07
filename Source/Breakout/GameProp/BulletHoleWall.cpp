// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/BulletHoleWall.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"
#include "Operations/MeshBoolean.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
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
	UE_LOG(LogTemp, Warning, TEXT("HP: %f"), Hp);
	Hp = 50.f;
	bDestroyed = false;
}
void ABulletHoleWall::ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Hp -= Damage;
	UE_LOG(LogTemp, Warning, TEXT("HP: %f"), Hp);
	if (Hp <= 0.f && !bDestroyed)
	{
		//저장 부셔진 조각들 계산해서 여기는 수정해야함
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				for (int k = 0; k < 1; ++k)
				{
					FVector Loc = FVector(i, j, k) * 30.f;
					FTransform DataATransform;
					FTransform SculptureTransform;
					SculptureTransform.SetLocation(Loc);
					SculptureTransform.SetScale3D(FVector(0.5, 0.5, 0.5));
					MeshDataStorage.Add(MeshBoolean(MeshDataA, DataATransform, SetRandomVertex(SculptureData, -3.f, 10.f, 0.1f), SculptureTransform, false));

				}
			}
		}

		//각각 조각들 설정
		for (int i = 0; i < MeshDataStorage.Num(); ++i)
		{
			FTransform AddTransform;
			UProceduralMeshComponent* TempComponent;
			TempComponent = Cast<UProceduralMeshComponent>(AddComponentByClass(UProceduralMeshComponent::StaticClass(), false, AddTransform, false));
			AddInstanceComponent(TempComponent);
			if (TempComponent)
			{
				TArray<FProcMeshTangent> Tangents = {};
				//부셔진 조각들이 물리 시뮬 가능하게 변경해줌 bUseComplexAsSimpleCollision를 false로 바꿔서 AddCollisionConvexMesh로 컬리션을 다시 만들어줌
				TempComponent->bUseComplexAsSimpleCollision = false;
				TempComponent->SetSimulatePhysics(true);
				TempComponent->AddCollisionConvexMesh(MeshDataStorage[i].Verts);
				TempComponent->CreateMeshSection_LinearColor(0, MeshDataStorage[i].Verts, MeshDataStorage[i].Tris, MeshDataStorage[i].Normals, MeshDataStorage[i].UVs, MeshDataStorage[i].Colors, Tangents, true);
				if (CurMaterial)
				{
					DynamicMaterial = UMaterialInstanceDynamic::Create(CurMaterial, this);
					if (DynamicMaterial)
						TempComponent->SetMaterial(0, DynamicMaterial);
				}
				ProceduralMesh->DestroyComponent();

				GetWorldTimerManager().SetTimer(
					DestroyTimer,
					this,
					&ABulletHoleWall::AllDestroy,
					3.f
				);
			}
		}
	}
}
void ABulletHoleWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABulletHoleWall::SetBulletHole(const FVector SweepResult)
{
	HitLoc = SweepResult;
	Sphere->SetWorldLocation(HitLoc);


	FTransform ATransform = ProceduralMesh->GetRelativeTransform();
	FTransform BTransform;
	BTransform.SetLocation(Sphere->GetRelativeTransform().GetLocation());
	BTransform.SetRotation(GetActorRotation().Quaternion());
	BTransform.SetScale3D(FVector(60.f, 0.2f, 0.2f));

	MeshDataA =MeshBoolean(MeshDataA, ATransform, SetRandomVertex(MeshDataB, -20.f, 20.f, 0.001), BTransform,true);
	//FTransform Temp;
	//Temp.SetLocation(FVector(0.f, 0.f, 0.f) - ProceduralMesh->GetComponentLocation());
	//Temp.SetRotation(ProceduralMesh->GetComponentQuat());
	//Temp.SetScale3D(FVector(1.f, 1.f, 1.f) / ProceduralMesh->GetRelativeScale3D());
	//TransformMeshData(MeshDataA, Temp, true, FVector(0.f, 0.f, 0.f));

	TArray<FProcMeshTangent> Tangents = {};
	ProceduralMesh->CreateMeshSection_LinearColor(0, MeshDataA.Verts, MeshDataA.Tris, MeshDataA.Normals, MeshDataA.UVs, MeshDataA.Colors,Tangents, true);
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
																	&BooleanOutput,Option);

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
	for (int x = 0; x < Data.Tris.Num(); x+=3)
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

	Result.Tris.SetNumUninitialized(TriNum*3);

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

	//Result.CountSections();

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
	int x = 0, l = res.Verts.Num(), nl = res.Normals.Num();
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
			if (!skiprot) { FVector& n = res.Normals[x]; n = rot.RotateVector(n); 
			}
		}
	}
	return newdata;
}

void ABulletHoleWall::AllDestroy()
{
	Destroy();
}

void ABulletHoleWall::GetMeshDataFromStaticMesh(UStaticMesh* Mesh, UPARAM(ref) FMeshData& Data, int32 LODIndex, int32 SectionIndex, bool GetAllSections)
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
		//if (!LOD.Sections.IsValidIndex(SectionIndex))
		//{
		//	Data.CountSections(); return;
		//}
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

		//if (!GetAllSections)
		//{
		//	Data.CountSections(); return;
		//}
		SectionIndex += 1;
		sec += 1;
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
			Already.Emplace(tCoord, Result.Verts[x]);
		}
	}
	return Result;
}
