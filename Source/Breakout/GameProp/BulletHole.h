// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryActors/GeneratedDynamicMeshActor.h"
#include "BulletHole.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ABulletHole : public AGeneratedDynamicMeshActor
{
	GENERATED_BODY()
	
public:
	ABulletHole();

	virtual void OnRebuildGeneratedMesh(UDynamicMesh* TargetMesh);
	virtual void ExecuteRebuildGeneratedMeshIfPending();
	virtual void BeiginPlay();
	UPROPERTY(EditAnywhere)
	float BoxDimensionX;
	float BoxDimensionY;
	float BoxDimensionZ;



};
