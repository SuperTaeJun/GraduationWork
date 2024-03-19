// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/CharacterController.h"
#include "SelectCharacterUi.generated.h"


/**
 * 
 */
UCLASS()
class BREAKOUT_API USelectCharacterUi : public UUserWidget
{
	GENERATED_BODY()


public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* Character1Button;

	UPROPERTY(meta = (BindWidget))
	class UButton* Character2Button;

	UPROPERTY(meta = (BindWidget))
	class UButton* Character3Button;

	UPROPERTY(meta = (BindWidget))
	class UButton* Character4Button;

	UFUNCTION()
	void Character1ButtonPressed();
	UFUNCTION()
	void Character2ButtonPressed();
	UFUNCTION()
	void Character3ButtonPressed();
	UFUNCTION()
	void Character4ButtonPressed();
private:
	ACharacterController* MyCharacterController;
};
