// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LoginHUD.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ALoginHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<class UUserWidget>LoginUiClass;

	TObjectPtr<class ULogin> LoginUi;

	void AddLoginUi();
protected:
	virtual void BeginPlay() override;
};
