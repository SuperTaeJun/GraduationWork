// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshUtility.h"
#include "MeshDescription.h"
#include "PropBase.generated.h"

UCLASS()
class BREAKOUT_API APropBase : public AActor
{
	GENERATED_BODY()
	
public:	
	APropBase();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Mesh")
	TObjectPtr<class UProceduralMeshComponent> ProceduralMesh;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<class USphereComponent>AreaSphere;
	TObjectPtr<class UProceduralMeshUtility> ProcMeshUtillity;

	//float Time = 0.f;
	//float MorphingSpeed = 0.3f;
	//float Cur = 0.f;
	//int32 RandValue;

};
