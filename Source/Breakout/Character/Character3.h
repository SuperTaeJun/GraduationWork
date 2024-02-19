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
	TObjectPtr<class UNiagaraComponent> NiagaraComp;

protected:
	virtual void Skill_S(const FInputActionValue& Value) override;
	virtual void Skill_E(const FInputActionValue& Value) override;

};
