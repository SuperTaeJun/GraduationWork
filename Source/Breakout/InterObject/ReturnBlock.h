// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReturnBlock.generated.h"

UCLASS()
class BREAKOUT_API AReturnBlock : public AActor
{
	GENERATED_BODY()
	
public:	
	AReturnBlock();
	UPROPERTY(EditAnywhere, Category = "Mesh")
	TObjectPtr<class UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> CollisionBox;
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnHit
	(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NomalImpulse, const FHitResult& Hit);

	bool bRetrun = false;
	FTimerHandle RetrunTimer;
	void RetrunStart() { bRetrun = true; }
private:
	TDoubleLinkedList<FrameData> FrameDatas;
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
	float MaxSaveTime = 15.f;
};

USTRUCT()
struct FrameData
{
	GENERATED_BODY()

	FVector Location;
	FRotator Rotation;
	float DeltaTime;

	FrameData()
	{
	};
	FrameData(FVector _Location, FRotator _Rotation, float _DeltaTime)
	{
		Location = _Location;
		Rotation = _Rotation;
		DeltaTime = _DeltaTime;
	};
};