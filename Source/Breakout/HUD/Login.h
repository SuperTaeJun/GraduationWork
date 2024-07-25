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
	TObjectPtr<class UButton>SignUp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableText>ID;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableText>Password;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableText>IP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton>Fail;

	UFUNCTION()
	void HoverLog();

	UFUNCTION()
	void PressLogin();
	UFUNCTION()
	void PressSignUp();

	UFUNCTION()
	void PressFail();
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ClickSound;

};
