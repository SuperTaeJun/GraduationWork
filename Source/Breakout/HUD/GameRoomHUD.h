// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameRoomHUD.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AGameRoomHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<class UUserWidget>SelectCharacterClass;

	TObjectPtr<class USelectCharacterUi> SelectCharacter;


	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<class UUserWidget>LoginUiClass;

	TObjectPtr<class ULogin> LoginUi;

	UPROPERTY(EditAnywhere, Category = "Player State")
	TSubclassOf<class UUserWidget>RoomListUiClass;
	TObjectPtr<class URoomListUi> RoomListUi;
	void AddLoginUi();
	void AddSelectCharacter();
	void AddRoomListUi();
protected:
	virtual void BeginPlay() override;

};
