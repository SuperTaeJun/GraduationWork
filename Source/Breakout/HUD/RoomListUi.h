// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoomListUi.generated.h"

/**
 * 
 */

struct RoomInfo
{
	int32 PageNum = 0;
	int32 NumOfUser = 0;
	FString RoomName = "EMPTY";
	bool bCanEnter = true;
};

UCLASS()
class BREAKOUT_API URoomListUi : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	virtual void NativeConstruct() override;
	TArray<RoomInfo> RoomList;
	int32 CurPageNum = 0;


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton>PrevButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton>NextButton;
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

};
