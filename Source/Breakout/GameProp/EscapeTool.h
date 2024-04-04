// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameProp/PropBase.h"
#include "EscapeTool.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AEscapeTool : public APropBase
{
	GENERATED_BODY()
	
public:
	AEscapeTool();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);

	void TransformMesh(float DeltaTime, bool Clamp, bool TransformReverse);
	void SetHideMesh();

	void SetbDetected(bool _bDetected) { bDetected = _bDetected; }
private:
	bool bDetected;

	UFUNCTION()
	virtual void OnSphereOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	UFUNCTION()
	void OnSphereEndOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	TObjectPtr<class ACharacterBase> OverlapedCharacter;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UWidgetComponent> PercentBar;

	void UpdatePercent(float Percent);

	//0 = 기본상태
	//1 = a->b로 보간중인 상태
	//2 = b->a로 다시 돌아가는 상태
	//이넘클래스로 다시 만들기 
	int32 bOverlap=0;
};
