#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Type/TurningType.h"
#include "CharacterBase.generated.h"

class UInputAction;

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


protected:
	virtual void BeginPlay() override;

	void UpdateSprintCamera(float DeltaTime);
	void UpdateStamina(float DeltaTime);

	//캐릭터 상태
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float Health = 100.f;
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float Stamina = 100.f;
	bool StaminaExhaustionState;
	ECharacterState CharacterState;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCharacterMovementComponent> Movement;

	ETurningInPlace TurningType;
	void TurnInPlace(float DeltaTime);

	void AimOffset(float DeltaTime);
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;


	//입력값
protected:
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
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Sprint_S(const FInputActionValue& Value);
	void Sprint_E(const FInputActionValue& Value);
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
