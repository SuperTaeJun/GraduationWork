// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/EscapeTool.h"
#include "Character/CharacterBase.h"
#include "ProceduralMeshComponent.h"
#include "Components/SphereComponent.h"

AEscapeTool::AEscapeTool()
{
}

void AEscapeTool::BeginPlay()
{
	if (AreaSphere)
	{
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AEscapeTool::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AEscapeTool::OnSphereEndOverlap);
	}

}

void AEscapeTool::TransformMesh(float DeltaTime)
{

	if (OverlapedCharacter)
	{
		if (Cur >= 1.f)
		{
			UE_LOG(LogTemp, Log, TEXT("CUR 1.F"));
			OverlapedCharacter->SetbCanObtainEscapeTool(true);
		}
	}

	Cur = FMath::Clamp
	(
		Time/*(((DegSin(Time * 180.f) * 1.1) + 1.0) / 2.f)*/,
		0.f,
		1.f
	);

	InterpMeshData(InterpData, Data1, Data2, Cur, false);
	ProceduralMesh->UpdateMeshSection_LinearColor(0, InterpData.Verts, InterpData.Normals, InterpData.UVs, InterpData.Colors, TArray<FProcMeshTangent>());

	Time = Time + (DeltaTime * MorphingSpeed);
}

void AEscapeTool::SetHideMesh()
{
	UE_LOG(LogTemp, Log, TEXT("TEST"));
	ProceduralMesh->SetHiddenInGame(true);
}

void AEscapeTool::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OverlapedCharacter = Cast<ACharacterBase>(OtherActor);
	if (OverlapedCharacter)
	{
		OverlapedCharacter->OverlappingEscapeTool = this;
	}
	//CharacterBase->SetbCanObtainEscapeTool(true);

//UE_LOG(LogTemp, Log, TEXT("OBTAIN"));
}

void AEscapeTool::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ProceduralMesh->bHiddenInGame==false)
	{
		ACharacterBase* characterbase = Cast<ACharacterBase>(OtherActor);

		characterbase->SetbCanObtainEscapeTool(false);
		characterbase->OverlappingEscapeTool = nullptr;

	}
}
