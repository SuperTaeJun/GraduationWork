// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ItemBase.h"
#include "HealthItem.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AHealthItem : public AItemBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
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

private:
	float IncreMaxHp = 10.f;
};
