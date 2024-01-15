// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ClientSocket.h"
#include "BOGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API ABOGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABOGameMode();
	virtual void Tick(float DeltaTime) ;
	virtual void BeginPlay();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

private:
	ClientSocket* Socket;
	bool bIsConnected;
};
