
#include "FX/ReplayFX.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/KismetMaterialLibrary.h"

AReplayFX::AReplayFX()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PoseableMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseableMesh"));
	RootComponent = PoseableMesh;
	ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_PoseMesh(TEXT("/Game/BP/Character/Test/SK_Character4.SK_Character4"));
	if (SK_PoseMesh.Succeeded())
	{
		PoseableMesh->SetSkeletalMesh(SK_PoseMesh.Object);
	}
	ConstructorHelpers::FObjectFinder<UMaterialInstance> M_GhostTail(TEXT("/Game/BP/Character/Test/NewMaterial_Inst.NewMaterial_Inst"));
	if (M_GhostTail.Succeeded())
	{
		GhostMaterial = M_GhostTail.Object;
		//UE_LOG(LogTemp, Log, TEXT("Material Good"));
	}
}

void AReplayFX::Init(USkeletalMeshComponent* Pawn)
{
	PoseableMesh->CopyPoseFromSkeletalComponent(Pawn);
	TArray<UMaterialInterface*> Mats = PoseableMesh->GetMaterials();

	for (int i = 0; i < Mats.Num(); i++)
	{
		GhostMaterials.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), GhostMaterial));
		PoseableMesh->SetMaterial(i, GhostMaterials[i]);
	}
	FadeCountDown = FadeOutTime;
	IsSpawned = true;
}

// Called when the game starts or when spawned
void AReplayFX::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AReplayFX::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (IsSpawned)
	//{
	//	FadeCountDown -= DeltaTime;
	//	for (int i = 0; i < GhostMaterials.Num(); i++)
	//	{
	//		GhostMaterials[i]->SetScalarParameterValue("Opacity", FadeCountDown / FadeOutTime);
	//	}
	//	if (FadeCountDown < 0)
	//	{
	//		Destroy();
	//	}
	//}
}

