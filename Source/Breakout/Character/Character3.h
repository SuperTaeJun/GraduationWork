// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"

#include "Character3.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ACharacter3 : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	ACharacter3();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UNiagaraComponent> NiagaraComp;
	//class UBOGameInstance* inst;
	//TObjectPtr<class UMaterialInstanceDynamic> DynamicMaterial;

protected:
	virtual void Skill_S(const FInputActionValue& Value) override;
	virtual void Skill_E(const FInputActionValue& Value) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UMaterialInstance> OldMaterial;

	
private:
	TObjectPtr<class UCharacterMovementComponent> MovementComp;
	float OldMaxWalkSpeed;
	float OldMaxAcceleration;
	bool bCancel = true;
	bool bGhost = false;
	bool bCoolTimeFinish = true;
	float GhostCoolChargeTime = 0.f;
	FTimerHandle GhostTimer;
	void GhostStart();
	void GhostEnd();
public:
	void ServerGhostStart();
	void ServerGhostEnd();
};
