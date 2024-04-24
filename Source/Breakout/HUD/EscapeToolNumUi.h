// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeToolNumUi.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API UEscapeToolNumUi : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Player1;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Player2;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Player1Ch;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Player2Ch;

};
