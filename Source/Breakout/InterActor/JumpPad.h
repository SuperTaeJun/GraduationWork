// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InterActor/InterActorBase.h"
#include "JumpPad.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AJumpPad : public AInterActorBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay();
public:
	UFUNCTION()
	virtual void OnBoxOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};