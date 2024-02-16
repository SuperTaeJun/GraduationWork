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
	TObjectPtr<class AController> Controller;

public:
	FORCEINLINE void SetSelectedSkill(ESelectedSkill Selected) { CurSelectedSKill = Selected; }
	FORCEINLINE ESelectedSkill GetSelectedSkill() { return CurSelectedSKill; }

	FORCEINLINE void SetIsReverse(bool _IsReverse) { bTimeReplay = _IsReverse; }
private:
	ESelectedSkill CurSelectedSKill;
	//Skill1
	TDoubleLinkedList<FCharacterFrameData> FrameDatas;
	void StoreFrameData(float DeltaTime);
	void Replay(float DeltaTime);
	bool bTimeReplay;
	//out of time data, cannot keep replay
	bool bOutOfData;
	//실제로 진행된시간
	float RunningTime;
	//각프레임마다의 델타타임
	float LeftRunningTime;
	float RightRunningTime;
	//저장된 각프레임마다에서 기록된 전체시간 
	float RecordedTime;
	float MaxSaveTime = 5.f;
	float Temp = 0.2f;
	//Skill2
};

USTRUCT(BluePrintType)
struct FCharacterFrameData
{
	GENERATED_BODY()
	
	FVector Location;
	float DeltaTime;

	FCharacterFrameData()
	{
	};
	FCharacterFrameData(FVector _Location, float _DeltaTime)
	{
		Location = _Location;
		DeltaTime = _DeltaTime;
	};
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