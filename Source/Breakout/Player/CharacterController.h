// Fill out your copyright notice in the Description page of Project Settings.


#pragma once
#include <queue>
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include <memory>
#include "CharacterController.generated.h"

class ClientSocket;
class CPlayer;
class CPlayerInfo;
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
	ACharacterController();
	
	int my_session_id;
	int other_session_id;
	int other_x;
	int other_y;
	int other_z;
	int count;
	
	//virtual void OnPossess(APawn* InPawn) override;
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
	// 초기 플레이어 저장
	void SetPlayerID(const int playerid) { id = playerid; }
	void SetPlayerInfo(CPlayerInfo* p_info) { 
		if (p_info != nullptr) 
			PlayerInfo = p_info; 
	}
	void SetInitPlayerInfo(const CPlayer& owner_player);
	//-----------------------------------------------------

	void UpdatePlayer(int input);
	//동기화 용
	void UpdateSyncPlayer();
	void SetNewCharacterInfo(std::shared_ptr<CPlayer*> InitPlayer);
	//Tick함수
	virtual void Tick(float DeltaTime);

	//void RecvNewPlayer(int sessionID, float x, float y, float z);
	//void SendPlayerPos(int id);

	// 스폰시킬 다른 캐릭터
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TSubclassOf<class ACharacter> ToSpawn;

	virtual void OnPossess(APawn* InPawn) override;
private:
	TObjectPtr<class AMainHUD> MainHUD;
	int id;
	bool bNewPlayerEntered = false;
	bool bInitPlayerSetting = false;
	ClientSocket* c_socket;
	CPlayerInfo* PlayerInfo;  
	CPlayer* initplayer;
	// 다른 캐릭터들의 정보
	std::queue<CPlayer*> NewPlayer;
};
