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

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

	//class UBOGameInstance* inst;
protected:
	virtual void Skill_S(const FInputActionValue& Value) override;
	virtual void Skill_E(const FInputActionValue& Value) override;

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UNiagaraComponent> NiagaraComp;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UArrowComponent>FXroc;

	TObjectPtr<class UCharacterMovementComponent> MovementComp;

	float OldMaxWalkSpeed;
	float OldMaxAcceleration;
	FRotator OldRotationRate;
	void DashSetup(float _MaxAcc, float _MaxWalk, FRotator _Rotation, bool _Visibillity);
	void DashFinishSetup();
	float DashSpeed;
	FTimerHandle DashTimer;

	bool bDash = false;
	bool bCoolTimeFinish = true;
	int32 DashPoint = 3;
	float DashCoolChargeTime = 0.f;
	FVector OldVelocity;
	void DashStart();
	void FinishDashTimer();
	void CoolTimeDashTimer();
public:
	void ServerDashSetup();
	void ServerNiagaraSync();
	void ServerDashFinish();
};
