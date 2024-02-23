// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Character4.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ACharacter4 : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
protected:
	virtual void Skill_S(const FInputActionValue& Value) override;
	virtual void Skill_E(const FInputActionValue& Value) override;
	
private:
	FTimerHandle TelpoTimer;
	FVector SavedLocation;
	bool TelepoChargeTime = true;
	bool bSaved = false;
	bool CanTelepo = false;
	int Toggle = 1;
	float CoolChargeTime = 0.f;
	void SetCanTelepo() { CanTelepo = true; 		Toggle += 1; }
	void SaveCurLocation();
	void SetLocation();
public:
	UPROPERTY(BlueprintReadWrite)
	class AReplayFX* GhostMesh;
};
