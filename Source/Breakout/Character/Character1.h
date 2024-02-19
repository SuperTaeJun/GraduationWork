// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Character1.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ACharacter1 : public ACharacterBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	
protected:
	virtual void Skill_S(const FInputActionValue& Value) override;
	virtual void Skill_E(const FInputActionValue& Value) override;


};
