// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Character2.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ACharacter2 : public ACharacterBase
{
	GENERATED_BODY()
public:
	ACharacter2();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);
protected:
	virtual void Skill_S(const FInputActionValue& Value) override;
	virtual void Skill_E(const FInputActionValue& Value) override;

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraComponent> NiagaraComp;

	bool bDash = false;
	bool bCoolTimeFinish = true;
	int32 DashPoint = 3;
	float DashCoolChargeTime = 0.f;
	FVector OldVelocity;
	FTimerHandle DashTimer;
	void DashStart();
	void FinishDashTimer();
	void CoolTimeDashTimer();
};
