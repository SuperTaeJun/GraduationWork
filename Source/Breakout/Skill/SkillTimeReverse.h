// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillTimeReverse.generated.h"

USTRUCT(BluePrintType)
struct FCharacterFrameData
{
	GENERATED_BODY()

	FTransform Transform;
	FRotator ControlRotation;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREAKOUT_API USkillTimeReverse : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USkillTimeReverse();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	TObjectPtr<class ACharacterBase> Character;
	TObjectPtr<class ACharacterController> Controller;
	FCharacterFrameData* FrameData;
};
