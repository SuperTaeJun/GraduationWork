// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidgetBlueprint.h"
#include "LoginUi.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ULoginUi : public UUserWidgetBlueprint
{
	GENERATED_BODY()
	
public:
	ULoginUi();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton>Login;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableTextBox>ID;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableTextBox>Password;

	void PressLogin();

};
