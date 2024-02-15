// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREAKOUT_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USkillComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	TObjectPtr<class ACharacterBase> Character;
	TObjectPtr<class ACharacterController> Controller;

public:
	FORCEINLINE void SetSelectedSkill(ESelectedSkill Selected) { CurSelectedSKill = Selected; }
	FORCEINLINE ESelectedSkill GetSelectedSkill() { return CurSelectedSKill; }

private:
	//Skill1
	FCharacterFrameData* FrameData;
	ESelectedSkill CurSelectedSKill;
	FTimerHandle Skill1Timer;
	int32 Cnt = 0;
	void StoreFrameData();
	void StoreTimer();
public:
	void Recall();
};

USTRUCT(BluePrintType)
struct FCharacterFrameData
{
	GENERATED_BODY()

	TArray<FTransform> Transform;
	FRotator ControlRotation;
	int32 MaxTransfirm;
};
UENUM(BlueprintType)
enum class ESelectedSkill : uint8
{
	E_Skill1 UMETA(DisplayName = "E_Skill1"),
	E_Skill2 UMETA(DisplayName = "E_Skill2"),
	E_Skill3 UMETA(DisplayName = "E_Skill3"),
	E_Skill4 UMETA(DisplayName = "E_Skill4"),

	ECS_DEFAULT UMETA(DisplayName = "Default")
};