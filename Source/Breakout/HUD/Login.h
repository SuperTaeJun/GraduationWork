// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Login.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ULogin : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton>Login;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableText>ID;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableText>Password;

	UFUNCTION()
	void PressLogin();
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ClickSound;

};
