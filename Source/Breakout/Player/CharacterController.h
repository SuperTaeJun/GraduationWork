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

	int my_session_id;
	int other_session_id;
	int other_x;
	int other_y;
	int other_z;
	int count;
	
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDStamina(float Stamina, float MaxStamina);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDEscapeTool(int32 EscapeTool);
	void SetHUDCrosshair(const struct FCrosshairPackage& Package);
	//skill
	void SetHUDSkill();
	//스킬 아이콘 투명도
	void SetHUDSkillOpacity(float Opacity);
	//skill1,3,4 쿨타임
	void SetHUDCool(float Cool, float MaxCool);
	//skill2 대쉬포인트
	void SetHUDCool(int32 Cool);
	void SetHUDCoolVisibility(bool bVisibility);
	void showWeaponSelect();
	void RecvNewPlayer(int sessionID, float x, float y, float z);
	void SendPlayerPos(int id);
	//void SetNewCharacterInfo(shared_ptr<CPlayer> InitPlayer);
	//동기화 용
	//void UpdateSyncPlayer();
	// 스폰시킬 다른 캐릭터
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class ACharacter> ToSpawn;
	virtual void OnPossess(APawn* InPawn) override;
private:
	TObjectPtr<class AMainHUD> MainHUD;
	bool bNewPlayerEntered = false;
	/*CPlayer initplayer;
	CPlayerInfo* PlayerInfo;*/
	//ClientSocket* c_socket;
		// 다른 캐릭터들의 정보
	//queue<shared_ptr<CPlayer>>	//NewPlayer;
};
