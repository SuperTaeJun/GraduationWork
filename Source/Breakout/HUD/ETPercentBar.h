// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ETPercentBar.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API UETPercentBar : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* PercentBar;

};
