// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchingUi.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API UMatchingUi : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WaitingText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ContingText;

	void SetWaitingText();
	void SetContingText(float Time);
};
