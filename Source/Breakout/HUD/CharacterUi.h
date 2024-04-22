// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterUi.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API UCharacterUi : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* StaminaBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* StaminaText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ToolAmount;

	UPROPERTY(meta = (BindWidget))
	class UImage* SkillImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SkillCool;
	UPROPERTY(meta = (BindWidget))
	class UImage* BojomugiImage;


	UPROPERTY(EditAnywhere)
	class UTexture2D* SkillIcon1;
	UPROPERTY(EditAnywhere)
	class UTexture2D* SkillIcon2;
	UPROPERTY(EditAnywhere)
	class UTexture2D* SkillIcon3;
	UPROPERTY(EditAnywhere)
	class UTexture2D* SkillIcon4;

	UPROPERTY(EditAnywhere)
	class UTexture2D* GrenadeImage;
	UPROPERTY(EditAnywhere)
	class UTexture2D* WallImage;
	UPROPERTY(EditAnywhere)
	class UTexture2D* BoobtTrapImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Player1;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Player2;
};
