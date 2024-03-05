// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/BulletHoleWall.h"
#include "Components/BoxComponent.h"
#include "ProceduralMeshComponent.h"

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
	//CollisionBox->OnComponentHit.AddDynamic(this, &ABulletHoleWall::OnHit);
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABulletHoleWall::OnOverlap);
	//ProceduralMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
}
void ABulletHoleWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABulletHoleWall::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("ONHIT"));
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
