// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Character1.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ACharacter1 : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	ACharacter1();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
protected:
	virtual void Destroyed();

	virtual void Skill_S(const FInputActionValue& Value) override;
	virtual void Skill_E(const FInputActionValue& Value) override;
	void Skill_T(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraSystem> TimeReplayNiagara;
	float NiagaraSpawnRate=0.15;
	float NiagaraSpawnSavedTime;
	//최대 저장시간
	UPROPERTY(EditAnywhere)
	float MaxSaveTime = 15.f;
private:
	/*UPROPERTY(EditAnywhere)
	TObjectPtr<USkeletalMesh> SkMeshAsset;*/

	TDoubleLinkedList<FCharacterFrameData> FrameDatas;
	void StoreFrameData(float DeltaTime);
	void Replay(float DeltaTime);
	bool bTimeReplay = false;
	//out of time data, cannot keep replay
	bool bOutOfData;
	//실제로 진행된시간
	float RunningTime;
	//각프레임마다의 델타타임
	float LeftRunningTime;
	float RightRunningTime;
	//저장된 각프레임마다에서 기록된 전체시간 
	float RecordedTime;
	float Temp = 0.2f;

	bool bCoolTimeFinish = true;
	float RecordedCoolTime = 0.f;

	void UpdateHUDCool(float Cool, float MaxCool);
	
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