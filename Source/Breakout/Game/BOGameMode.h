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
	virtual void PlayerRemove(class ACharacterBase* RemovedCharacter, class ACharacterController* RemovedCharacterController, class ACharacterController* AttackerController);
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	TSubclassOf<class ACharacterBase>Character1;
	TSubclassOf<class ACharacterBase>Character2;
	TSubclassOf<class ACharacterBase>Character3;
	TSubclassOf<class ACharacterBase>Character4;
//private:
//	ClientSocket* m_Socket;
//	bool connect;
};
