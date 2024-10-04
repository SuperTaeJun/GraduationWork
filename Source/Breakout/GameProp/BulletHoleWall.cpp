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

	ProcMeshUtillity = CreateDefaultSubobject<UProceduralMeshUtility>(TEXT("ProcMeshUtillity"));

	Hp = 50.f;
	bDestroyed = false;
}

void ABulletHoleWall::BeginPlay()
{
	Super::BeginPlay();
	OnTakeAnyDamage.AddDynamic(this, &ABulletHoleWall::ReciveDamage);

	//부셔진후 조각
	//SculptureMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/BreakoutAsset/BulletHole/SM_Scul.SM_Scul")).Object;
	ProcMeshUtillity->GetMeshDataFromStaticMesh(SculptureMesh, SculptureData, 0);
	ProcMeshUtillity->SetColorData(SculptureData, FLinearColor::Black);

	//벽
	//MeshA = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/BreakoutAsset/BulletHole/block.block")).Object;
	ProcMeshUtillity->GetMeshDataFromStaticMesh(MeshA, MeshDataA, 0);
	ProcMeshUtillity->SetColorData(MeshDataA, FLinearColor::Black);

	UE_LOG(LogTemp, Warning, TEXT("MeshA Num : %d"), MeshDataA.Verts.Num());

	//구멍 모양
	//MeshB = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Game/BreakoutAsset/BulletHole/SM_BulletHole.SM_BulletHole")).Object;
	ProcMeshUtillity->GetMeshDataFromStaticMesh(MeshB, MeshDataB, 0);
	ProcMeshUtillity->SetColorData(MeshDataB, FLinearColor::Black);

	TArray<FProcMeshTangent> Temp{};
	ProceduralMesh->CreateMeshSection_LinearColor
	(
		0,
		MeshDataA.Verts,
		MeshDataA.Tris,
		MeshDataA.Normals,
		MeshDataA.UVs,
		MeshDataA.Colors,
		Temp,
		true
	);

	ProceduralMesh->SetMaterial(0, CurMaterial);


	ResetMeshData = MeshDataA;
}
void ABulletHoleWall::ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Hp -= Damage;

	if (Hp <= 0.f && !bDestroyed)
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				//for (int k = 0; k < 1; ++k)
				//{
				FVector Loc = FVector(i, j, 0) * 30.f;
				FTransform DataATransform;
				FTransform SculptureTransform;
				SculptureTransform.SetLocation(Loc);
				SculptureTransform.SetScale3D(FVector(0.5, 0.5, 0.5));
				MeshDataStorage.Add(ProcMeshUtillity->MeshBoolean(MeshDataA, DataATransform, ProcMeshUtillity->SetRandomVertex(SculptureData, -3.f, 10.f, 0.1f), SculptureTransform, false));

				//}
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
		GetWorldTimerManager().SetTimer(DestroyTimer, this, &ABulletHoleWall::AllDestroy, 4.f);
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

	MeshDataA = ProcMeshUtillity->MeshBoolean(MeshDataA, ATransform, ProcMeshUtillity->SetRandomVertex(MeshDataB, -20.f, 20.f, 0.001), BTransform, true);

	TArray<FProcMeshTangent> Tangents = {};
	ProceduralMesh->CreateMeshSection_LinearColor(0, MeshDataA.Verts, MeshDataA.Tris, MeshDataA.Normals, MeshDataA.UVs, MeshDataA.Colors, Tangents, true);
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
