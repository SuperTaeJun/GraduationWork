// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameRoomMode.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API AGameRoomMode : public AGameMode
{
	GENERATED_BODY()
public:
	AGameRoomMode();

	virtual void Tick(float Delta) override;
};
