// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RespawnSelect.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API URespawnSelect : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void Respawn1Pressed();
	UFUNCTION()
	void Respawn2Pressed();
	UFUNCTION()
	void Respawn3Pressed();
	UFUNCTION()
	void Respawn4Pressed();
public:
	UPROPERTY(meta = (BindWidget))
	class UButton* Respawn1;

	UPROPERTY(meta = (BindWidget))
	class UButton* Respawn2;

	UPROPERTY(meta = (BindWidget))
	class UButton* Respawn3;

	UPROPERTY(meta = (BindWidget))
	class UButton* Respawn4;

private:
	void SelectButton(FName TagName);
};
