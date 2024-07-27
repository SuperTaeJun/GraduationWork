// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Skill4StartActor.generated.h"

UCLASS()
class BREAKOUT_API ASkill4StartActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASkill4StartActor();

	void Init(class USkeletalMeshComponent* Pawn);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	class UPoseableMeshComponent* PoseableMesh;
	class UMaterialInstance* GhostMaterial;
	TArray<class UMaterialInstanceDynamic*> Materials;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	bool bTimerStart = false;
	FTimerHandle TelpoTimer;
	void Dead();
};
