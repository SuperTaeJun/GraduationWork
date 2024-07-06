// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/CharacterController.h"
#include "SelectWeaponUi.generated.h"

/**
 * 
 */
UCLASS()
class BREAKOUT_API USelectWeaponUi : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void RifleButtonPressed();
	UFUNCTION()
	void ShotgunButtonPressed();
	UFUNCTION()
	void LancherButtonPressed();
public:
	UPROPERTY(meta = (BindWidget))
	class UButton* RifleButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ShotgunButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* LancherButton;

	UPROPERTY(EditAnywhere, Category = "Combat System", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AWeaponBase> Rifle;

	UPROPERTY(EditAnywhere, Category = "Combat System", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AWeaponBase> ShotGun;

	UPROPERTY(EditAnywhere, Category = "Combat System", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AWeaponBase> Lancher;
private:
	ACharacterController* MyCharacterController;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ClickSound;

};
