#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Type/TurningType.h"
#include "CharacterBase.generated.h"

class UInputAction;
#define DEFAULTCAMERALENGTH 200
#define SPRINTCAMERALENGTH 100

UCLASS()
class BREAKOUT_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ACharacterBase();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE ETurningInPlace GetTurningType() { return TurningType; }

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	class AWeaponBase* GetCurWeapon() const { return CurWeapon; }

	/*float GetAO_Yaw();
	float GetAO_Pitch();*/
	int		_SessionId;

	EWeaponType CurWeaponType;
protected:
	virtual void BeginPlay() override;

	void UpdateSprintCamera(float DeltaTime);
	void UpdateStamina(float DeltaTime);
	void SetHUDCrosshair(float DeltaTime);
	void UpdateHpHUD();
	void UpdateStaminaHUD();
	void UpdateObtainedEscapeTool();

	//캐릭터 상태
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float Health = 100.f;
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxStamina = 100.f;
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float Stamina = 100.f;
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float DeadTime = 4.f;

	bool StaminaExhaustionState;

	int32 GrendeNum=10;
	int32 WallGrendeNum;
	int32 BoobyTrapNum;

	ECharacterState CharacterState;
	EBojoMugiType BojoMugiType;
	int32 ObtainedEscapeToolNum;

	FVector SWAimLastLoc;
public:
	void SetWeapon(TSubclassOf<class AWeaponBase> Weapon, FName SocketName);
	void SetWeaponUi();
	void SetRespawnUi();
	void SetbInRespon(bool _bInRespon) { bInRespon = _bInRespon; }
	bool GetbInRespon() { return bInRespon; }
	void SetbShowSelect(bool _bShowSelect) {bShowSelectUi = _bShowSelect;}
	void SetbCanObtainEscapeTool(bool _bCanObtain);
	void SetHealth(float DamagedHp);

	class AWeaponBase* GetWeapon() { return CurWeapon; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE bool GetbFfirePressed() const { return bFirePressed; }
	FORCEINLINE float MaxGetHealth() const { return MaxHealth; }
	FORCEINLINE float GetStamina() const { return Stamina; }
	FORCEINLINE float MaxGetStamina() const { return MaxStamina; }
	void PlayFireActionMontage();
	FVector GetHitTarget() { return HitTarget; }
	UFUNCTION()
	void ReciveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	TObjectPtr<class AEscapeTool> OverlappingEscapeTool;

	void GrandeThrow();
	void GrandeAim();

	UFUNCTION(BlueprintCallable)
	void GrandeThrowFinish();

	UFUNCTION(BlueprintCallable)
	void SpawnGrenade();

	void SetSpawnGrenade(TSubclassOf<class AProjectileBase> Projectile);

	void Dead();
	FTimerHandle DeadTimer;
	//int p_id; // player own id
protected:

	UPROPERTY(EditAnywhere, Category = Arrow)
	TObjectPtr<class UArrowComponent> PathSorce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UNiagaraComponent> Aim;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCharacterMovementComponent> Movement;

	TObjectPtr<class AWeaponBase> CurWeapon;

	TObjectPtr<class AMainHUD> MainHUD;
	TObjectPtr<class ACharacterController> MainController;

	UPROPERTY(EditAnywhere, Category = Animation)
	TObjectPtr<class UAnimMontage> FireActionMontage;
	UPROPERTY(EditAnywhere, Category = Animation)
	TObjectPtr<class UAnimMontage> GrenadeMontage;

	FName RightSocketName;

	//수류탄
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UStaticMeshComponent> Grenade;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectileBase> GrenadeClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectileBase> WallClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectileBase> BoobyTrapClass;

	//idle turn
	ETurningInPlace TurningType;
	void TurnInPlace(float DeltaTime);

	//에임오프셋
	void AimOffset(float DeltaTime);
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	//fire
	bool bCanFire;
	bool bFirePressed;
	FTimerHandle FireTimer;
	void StartFireTimer();
	void FireTimerFinished();
	void Fire();
	void FirePressd(bool _Pressd);
	void TraceUnderCrossHiar(FHitResult& TraceHitResult);


	
	bool bInRespon;
	bool bShowSelectUi;
	bool bCanObtainEscapeTool;

	//조준선
	UPROPERTY(EditAnywhere, Category = Crosshair)
	TObjectPtr<class UTexture2D> CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category = Crosshair)
	TObjectPtr<class UTexture2D> CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category = Crosshair)
	TObjectPtr<class UTexture2D> CrosshairsRight;
	UPROPERTY(EditAnywhere, Category = Crosshair)
	TObjectPtr<class UTexture2D> CrosshairsTop;
	UPROPERTY(EditAnywhere, Category = Crosshair)
	TObjectPtr<class UTexture2D> CrosshairsBottom;
	//크로스헤어기준으로 사거리만큼 조준했을때 마지막 충돌위치
	FVector HitTarget;

	//입력값
protected:
	bool CanJump=true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<class UInputMappingContext> DefalutMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> SprintAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> FireAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> AimAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> SkillAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> InterAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> ReRoadAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> GrandeFireAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> SelectGrandeAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> SelectWallAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> SelectTrapAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> DetectAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Sprint_S(const FInputActionValue& Value);
	void Sprint_E(const FInputActionValue& Value);
	void Fire_S(const FInputActionValue& Value);
	void Fire_E(const FInputActionValue& Value);
	void Inter(const FInputActionValue& Value);
	void EToolTranfrom(const FInputActionValue& Value);
	void Reroad(const FInputActionValue& Value);
	void GrandeFire_Aiming(const FInputActionValue& Value);
	void GrandeFire(const FInputActionValue& Value);
	void SelectGrande(const FInputActionValue& Value);
	void SelectWall(const FInputActionValue& Value);
	void SelectTrap(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	void StopJump(const FInputActionValue& Value);
	virtual void Skill_S(const FInputActionValue& Value);
	virtual void Skill_E(const FInputActionValue& Value);
	void Detect_S(const FInputActionValue& Value);
	void Detect_E(const FInputActionValue& Value);

public:
	//서버랑 연동하는 함수들
	void SpawnBeam(FVector StartBeam, FVector EndBeam);
	void SpawnHitImpact(FVector HitLoc, FRotator HitRot);
	bool bStarted;
	FTimerHandle StartHandle;
	void StartGame();
	
};

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_IDLE UMETA(DisplayName = "Idle"),
	ECS_RUN UMETA(DisplayName = "Run"),
	ECS_SPRINT UMETA(DisplayName = "Sprint"),
	ECS_FALLING UMETA(DisplayName = "Falling"),

	ECS_DEFAULT UMETA(DisplayName = "Default")
};

UENUM(BlueprintType)
enum class EBojoMugiType : uint8
{
	E_Grenade UMETA(DisplayName = "Grenade"),
	E_Wall UMETA(DisplayName = "Wall"),
	E_BoobyTrap UMETA(DisplayName = "BoobyTrap"),

	ECS_DEFAULT UMETA(DisplayName = "Default")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	E_Rifle UMETA(DisplayName = "Rifle"),
	E_Shotgun UMETA(DisplayName = "Shotgun"),
	E_Launcher UMETA(DisplayName = "Launcher"),

	ECS_DEFAULT UMETA(DisplayName = "Default")
};
