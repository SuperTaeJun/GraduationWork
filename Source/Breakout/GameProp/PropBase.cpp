

#include "GameProp/PropBase.h"
#include "ProceduralMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"
APropBase::APropBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	SetRootComponent(AreaSphere);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	ProceduralMesh->SetupAttachment(RootComponent);

	ProcMeshUtillity = CreateDefaultSubobject<UProceduralMeshUtility>(TEXT("ProcMeshUtillity"));
}

void APropBase::BeginPlay()
{
	Super::BeginPlay();

}

void APropBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

