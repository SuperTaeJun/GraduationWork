// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReplayFX.generated.h"

UCLASS()
class BREAKOUT_API AReplayFX : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	virtual void BeginPlay() override;
	AReplayFX();
	void Init(USkeletalMeshComponent* Pawn);

private:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	TObjectPtr<class UPoseableMeshComponent> PoseableMesh;
	TObjectPtr<class UMaterialInstance> GhostMaterial;
	TArray<UMaterialInstanceDynamic*> GhostMaterials;

	bool IsSpawned = false;
	float FadeCountDown;
	float FadeOutTime = 1.0f;
};
