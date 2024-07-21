// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

UCLASS()
class BREAKOUT_API AItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemBase();

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraComponent> ItemNiagaraMesh;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> CollisionBox;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraSystem>ObtainNiagara;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ObtainSound;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
};
