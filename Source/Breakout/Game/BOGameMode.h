// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
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
	//virtual void BeginPlay() override;

//private:
//	ClientSocket* m_Socket;
//	bool connect;
};
