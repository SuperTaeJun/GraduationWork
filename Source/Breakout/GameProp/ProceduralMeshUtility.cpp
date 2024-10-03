#include "GameProp/ProceduralMeshUtility.h"
#include "ProceduralMeshComponent.h"
#include "Operations/MeshBoolean.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/ProjectileBase.h"
UProceduralMeshUtility::UProceduralMeshUtility()
{
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UProceduralMeshUtility::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UProceduralMeshUtility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UProceduralMeshUtility::UnifyTri(MeshData& MeshData)
{
	// 방문한 정점을 추적
	TMap<int, int> VisitedVertices = {};
	int CurVertexCount = MeshData.Verts.Num();

	// 정점 속성 여부 체크
	bool bHasNormals = MeshData.Normals.Num() >= CurVertexCount;
	bool bHasUVs = MeshData.UVs.Num() >= CurVertexCount;
	bool bHasColors = MeshData.Colors.Num() >= CurVertexCount;
	bool bHasSections = MeshData.Sects.Num() >= CurVertexCount;

	for (int TriangleIndex = 0; TriangleIndex < MeshData.Tris.Num(); ++TriangleIndex)
	{
		int VertexIndex = MeshData.Tris[TriangleIndex];

		// 중복된 정점 처리
		if (!VisitedVertices.Contains(VertexIndex))
		{
			VisitedVertices.Emplace(VertexIndex, 1);
		}
		else
		{
			// 중복된 정점을 복제
			MeshData.Verts.Emplace(MeshData.Verts[VertexIndex]);

			if (bHasNormals)
				MeshData.Normals.Emplace(MeshData.Normals[VertexIndex]);
			if (bHasUVs)
				MeshData.UVs.Emplace(MeshData.UVs[VertexIndex]);
			if (bHasColors)
				MeshData.Colors.Emplace(MeshData.Colors[VertexIndex]);
			if (bHasSections)
				MeshData.Sects.Emplace(MeshData.Sects[VertexIndex]);

			MeshData.Tris[TriangleIndex] = CurVertexCount++;
		}
	}

	// 기존 데이터 저장해두기
	TArray<FVector> OriginalVerts = MeshData.Verts;
	TArray<FVector> OriginalNormals = MeshData.Normals;
	TArray<FVector2D> OriginalUVs = MeshData.UVs;
	TArray<FLinearColor> OriginalColors = MeshData.Colors;
	TArray<int32> OriginalTris = MeshData.Tris;

	// 정점 데이터 재정렬
	MeshData.Verts.Empty();
	MeshData.Tris.Empty();
	MeshData.Normals.Empty();
	MeshData.UVs.Empty();
	MeshData.Colors.Empty();

	for (int TriangleIndex = 0; TriangleIndex < OriginalTris.Num(); ++TriangleIndex)
	{
		MeshData.Verts.Emplace(OriginalVerts[OriginalTris[TriangleIndex]]);

		if (bHasNormals)
			MeshData.Normals.Emplace(OriginalNormals[OriginalTris[TriangleIndex]]);
		if (bHasUVs)
			MeshData.UVs.Emplace(OriginalUVs[OriginalTris[TriangleIndex]]);
		if (bHasColors)
			MeshData.Colors.Emplace(OriginalColors[OriginalTris[TriangleIndex]]);

		MeshData.Tris.Emplace(TriangleIndex);
	}
}

void UProceduralMeshUtility::GetMeshDataFromStaticMesh(UStaticMesh* Mesh, MeshData& Data ,int32 SectionIndex)
{
	int32 VertexCount = 0, SectionVertexIndex = 0, VertexIndex = 0, SectionID = 0;

	int32* NewIndexPtr = nullptr;
	if (Mesh == nullptr || Mesh->GetRenderData() == nullptr || !Mesh->GetRenderData()->LODResources.IsValidIndex(0))
	{
		return;
	}
	Data.Clear();

	while (true)
	{
		// 현재 LOD에 대한 리소스를 가져옴
		FStaticMeshLODResources& LOD = Mesh->GetRenderData()->LODResources[0];
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
		uint32 NumIndices = Indices.Num();

		bool bHasNomal = LOD.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices() >= LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		bool bHasUvs = LOD.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices() >= LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		bool bHasColors = LOD.VertexBuffers.ColorVertexBuffer.GetNumVertices() >= LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();


		for (TriangleIndex = FirstIndex; TriangleIndex < LastIndex; ++TriangleIndex)
		{
			if (TriangleIndex < NumIndices)
			{
				VertexIndex = Indices[TriangleIndex];
				NewIndexPtr = MeshToSectionVertMap.Find(VertexIndex);

				if (NewIndexPtr)
				{
					// 이미 매핑된 버텍스 인덱스 사용
					SectionVertexIndex = *NewIndexPtr;
				}
				else
				{
					// 새로운 버텍스 데이터를 수집
					Data.Verts.Emplace(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex));
					if(bHasNomal)
						Data.Normals.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex));
					if(bHasUvs)
						Data.UVs.Emplace(LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, 0));
					if(bHasColors)
						Data.Colors.Emplace(LOD.VertexBuffers.ColorVertexBuffer.VertexColor(VertexIndex));

					Data.Sects.Emplace(SectionID);
					

					// 새 버텍스 매핑 추가
					SectionVertexIndex = VertexCount;
					MeshToSectionVertMap.Emplace(VertexIndex, VertexCount);
					++VertexCount;
				}
				Data.Tris.Emplace(SectionVertexIndex);
			}

		}
		SectionIndex += 1;
		SectionID += 1;
	}
}

void UProceduralMeshUtility::SetColorData(MeshData& Data, FLinearColor Color)
{
	Data.Colors = {};
	Data.Colors.SetNumUninitialized(Data.Verts.Num());
	for (int x = 0; x < Data.Verts.Num(); ++x)
	{
		Data.Colors[x] = Color;
	}
}

void UProceduralMeshUtility::InterpMeshData(MeshData& OutData, MeshData& SourceDataA, MeshData& SourceDataB, float Alpha, bool bClamp)
{
	int VertexCount = 0;
	int CurrentVertexCount = OutData.Verts.Num();
	int SourceVertexCountA = SourceDataA.Verts.Num();
	int SourceVertexCountB = SourceDataB.Verts.Num();

	if (CurrentVertexCount <= 0 || SourceVertexCountA <= 0 || SourceVertexCountB <= 0)
	{
		return;
	}

	// Alpha 값이 클램프 범위에 있는지 확인
	if (bClamp)
	{
		if (Alpha <= 0.0f)
		{
			// Alpha가 0 이하일 경우 SourceDataA 사용
			if (OutData.Verts[0] != SourceDataA.Verts[0]) { OutData = SourceDataA; }
			return;
		}
		if (Alpha >= 1.0f)
		{
			// Alpha가 1 이상일 경우 SourceDataB 사용
			if (OutData.Verts[0] != SourceDataB.Verts[0]) { OutData = SourceDataB; }
			return;
		}
	}

	// 보간할 최대 버텍스 수 결정
	int MaxLerpCount = std::min(CurrentVertexCount, std::min(SourceVertexCountA, SourceVertexCountB));
	const bool bHasNormals = (OutData.Normals.Num() >= MaxLerpCount && SourceDataA.Normals.Num() >= MaxLerpCount && SourceDataB.Normals.Num() >= MaxLerpCount);
	const bool bHasUVs = (OutData.UVs.Num() >= MaxLerpCount && SourceDataA.UVs.Num() >= MaxLerpCount && SourceDataB.UVs.Num() >= MaxLerpCount);
	const bool bHasColors = (OutData.Colors.Num() >= MaxLerpCount && SourceDataA.Colors.Num() >= MaxLerpCount && SourceDataB.Colors.Num() >= MaxLerpCount);

	// 각 버텍스에 대해 보간 수행
	for (int VertexIndex = 0; VertexIndex < CurrentVertexCount; ++VertexIndex)
	{
		int SourceIndexB = VertexIndex;

		// SourceDataB의 인덱스가 유효하지 않은 경우 조정
		if (SourceVertexCountB < CurrentVertexCount && SourceIndexB >= SourceVertexCountB)
		{
			SourceIndexB = (SourceIndexB % SourceVertexCountB) / 3;
		}

		OutData.Verts[VertexIndex] = SpiralCustomLerp(SourceDataA.Verts[VertexIndex], SourceDataB.Verts[SourceIndexB], Alpha, 3.f, 30.f);
		if (bHasNormals)
		{
			OutData.Normals[VertexIndex] = SpiralCustomLerp(SourceDataA.Normals[VertexIndex], SourceDataB.Normals[SourceIndexB], Alpha, 3.f, 30.f);
			OutData.Normals[VertexIndex].Normalize();
		}
		if (bHasUVs)
		{
			OutData.UVs[VertexIndex] = FMath::Lerp(SourceDataA.UVs[VertexIndex], SourceDataB.UVs[SourceIndexB], Alpha);
		}
		if (bHasColors)
		{
			OutData.Colors[VertexIndex] = FMath::Lerp(SourceDataA.Colors[VertexIndex], SourceDataB.Colors[SourceIndexB], Alpha);
		}
	}

}

MeshData UProceduralMeshUtility::MeshBoolean(MeshData DataA, FTransform TransformA, MeshData DataB, FTransform TransformB, bool OptionType)
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
	UE::Geometry::FMeshBoolean Boolean(&DMeshA, ConvertedTransformA, &DMeshB, ConvertedTransformB, &BooleanOutput, Option);

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

	return 	ConverToMeshData(BooleanOutput, DataA);
}

MeshData UProceduralMeshUtility::SetRandomVertex(MeshData& Data, float Min, float Max, float Tolerance)
{
	MeshData Result = Data;
	TMap<FVector, FVector> Already = {};
	Tolerance = 1.0f / Tolerance;
	FVector tCoord;

	for (int x = 0; x < Data.Verts.Num(); ++x)
	{
		tCoord = FVector(Result.Verts[x].X * Tolerance, Result.Verts[x].Y * Tolerance, Result.Verts[x].Z * Tolerance);

		//이미 했던건지 확인
		if (Already.Contains(tCoord))
		{
			Result.Verts[x] = Already[tCoord];
		}
		else
		{
			if (Data.Normals.IsValidIndex(x))
			{
				Result.Verts[x] = Data.Verts[x] + Data.Normals[x] * FMath::RandRange(Min, Max) + FMath::VRand() * FMath::RandRange(Min, Max);
			}
			else
			{
				Result.Verts[x] = Data.Verts[x] + FMath::VRand() * FMath::RandRange(Min, Max);
			}
			//안했던거는 맵에 추가
			Already.Emplace(tCoord, Result.Verts[x]);
		}
	}
	return Result;
}

MeshData UProceduralMeshUtility::TransformMeshData(MeshData& Data, FTransform Transform, bool InPlace, FVector Pivot)
{
	MeshData newdata;
	MeshData* pres = &newdata;
	if (InPlace) { pres = &Data; }
	else { newdata = Data; }
	MeshData& res = *pres;
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



FVector UProceduralMeshUtility::SpiralCustomLerp(FVector& A, FVector& B, float& Alpha, float SpiralTurns, float Radius)
{
	FVector LinearInterpolatedPoint = FMath::Lerp(A, B, Alpha);

	FVector Direction = (B - A).GetSafeNormal();

	FVector OrthogonalVector1 = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
	FVector OrthogonalVector2 = FVector::CrossProduct(Direction, OrthogonalVector1).GetSafeNormal();

	float Angle = Alpha * SpiralTurns * 2.0f * PI;

	FVector SpiralOffset = (OrthogonalVector1 * FMath::Cos(Angle) + OrthogonalVector2 * FMath::Sin(Angle)) * Radius * (1.0f - Alpha);

	FVector SpiralInterpolatedPoint = LinearInterpolatedPoint + SpiralOffset;

	return SpiralInterpolatedPoint;
}

UE::Geometry::FDynamicMesh3 UProceduralMeshUtility::ConvertToFDynamicMesh3(MeshData& Data)
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

MeshData UProceduralMeshUtility::ConverToMeshData(UE::Geometry::FDynamicMesh3& Input, MeshData& Output)
{
	MeshData Result = Output;
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

FTransform3d UProceduralMeshUtility::ConvertToFTransform3d(FTransform Input)
{
	const FVector& Location = Input.GetLocation();
	const FVector& Scale = Input.GetScale3D();

	return FTransform3d(FQuat(Input.GetRotation()),
		FVector3d(Location.X, Location.Y, Location.Z),
		FVector3d(Scale.X, Scale.Y, Scale.Z)
	);
}

