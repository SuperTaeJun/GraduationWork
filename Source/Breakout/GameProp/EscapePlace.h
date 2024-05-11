// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EscapePlace.generated.h"

UCLASS()
class BREAKOUT_API AEscapePlace : public AActor
{
	GENERATED_BODY()
	
public:	
	AEscapePlace();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> AircraftMesh;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> CollisionBox;

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
	UFUNCTION()
	void OnBoxEndOverlap
	(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

};
