// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoomListUi.generated.h"

/**
 * 
 */

UCLASS()
class BREAKOUT_API URoomListUi : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	virtual void NativeConstruct() override;

	TObjectPtr<class UBOGameInstance> Inst;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton>SlotOne;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton>SlotTwo;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton>SlotThree;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton>SlotFour;

	UFUNCTION()
	void SlotOnePress();
	UFUNCTION()
	void SlotTwoPress();
	UFUNCTION()
	void SlotThreePress();
	UFUNCTION()
	void SlotFourPress();

	UFUNCTION()
	void SlotOneHoverd();
	UFUNCTION()
	void SlotTwoHoverd();
	UFUNCTION()
	void SlotThreeHoverd();
	UFUNCTION()
	void SlotFourHoverd();

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ClickSound;

};
