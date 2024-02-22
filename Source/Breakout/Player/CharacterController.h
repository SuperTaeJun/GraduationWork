// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "ClientSocket.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "CharacterController.generated.h"
/**
 * 
 */
UCLASS()
class BREAKOUT_API ACharacterController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	ClientSocket* connect_player;
	int my_session_id;
	int other_session_id;
	int other_x;
	int other_y;
	int other_z;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDStamina(float Stamina, float MaxStamina);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDEscapeTool(int32 EscapeTool);
	void SetHUDCrosshair(const struct FCrosshairPackage& Package);
	void showWeaponSelect();
	void RecvNewPlayer(int sessionID, float x, float y, float z);
	void SendPlayerPos(int id);
	//동기화 용
	void UpdateSyncPlayer();
	// 스폰시킬 다른 캐릭터
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class ACharacter> ToSpawn;
	virtual void OnPossess(APawn* InPawn) override;
private:
	TObjectPtr<class AMainHUD> MainHUD;
	bool bNewPlayerEntered;
	//ClientSocket* mysocket;
};
