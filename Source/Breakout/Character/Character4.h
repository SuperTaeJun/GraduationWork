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
	ACharacter4();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);
protected:
	virtual void Skill_S(const FInputActionValue& Value) override;
	virtual void Skill_E(const FInputActionValue& Value) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UNiagaraComponent> NiagaraComp;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ANiagaraActor> NiagaraActor;

	class ANiagaraActor* Temp;
private:
	FTimerHandle TelpoTimer;
	FVector SavedLocation;
	bool TelepoChargeTime = true;
	bool bSaved = false;
	float CoolChargeTime = 0.f;
	void SaveCurLocation();
	void SetLocation();

};
