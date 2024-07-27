// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/Skill4StartActor.h"
#include "TimerManager.h"
#include "Components/PoseableMeshComponent.h"
#include "Kismet/KismetMaterialLibrary.h"

ASkill4StartActor::ASkill4StartActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PoseableMesh = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("POSEABLEMESH"));
	RootComponent = PoseableMesh;
	ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_PoseMesh(TEXT("/Game/BreakoutAsset/Character/Character4/4_model.4_model"));
	if (SK_PoseMesh.Succeeded())
	{
		PoseableMesh->SetSkeletalMesh(SK_PoseMesh.Object);
	}
	ConstructorHelpers::FObjectFinder<UMaterialInstance> M_GhostTail(TEXT("/Game/Niagara/SKill/Skill4/M_SkillActorGhost_Inst.M_SkillActorGhost_Inst"));
	if (M_GhostTail.Succeeded())
	{
		GhostMaterial = M_GhostTail.Object;
	}

}

void ASkill4StartActor::Init(USkeletalMeshComponent* Pawn)
{
	PoseableMesh->CopyPoseFromSkeletalComponent(Pawn);
	TArray<UMaterialInterface*> Mats = PoseableMesh->GetMaterials();

	for (int i = 0; i < Mats.Num(); i++)
	{
		Materials.Add(UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), GhostMaterial));
		PoseableMesh->SetMaterial(i, Materials[i]);
	}

}

void ASkill4StartActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASkill4StartActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bTimerStart)
	{
		GetWorld()->GetTimerManager().SetTimer(TelpoTimer, this, &ASkill4StartActor::Dead, 0.5f, false);
		bTimerStart = false;
	}
}

void ASkill4StartActor::Dead()
{
	Destroy();
}
