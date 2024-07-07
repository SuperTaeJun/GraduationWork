// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/CharacterController.h"
#include "SelectCharacterUi.generated.h"


/**
 * 
 */
UCLASS()
class BREAKOUT_API USelectCharacterUi : public UUserWidget
{
	GENERATED_BODY()


public:
	virtual void NativeConstruct() override;
	void SetAllCharacterMeshWithTag();
	TObjectPtr<class ASkeletalMeshActor> Character1Mesh;
	TObjectPtr<class ASkeletalMeshActor> Character2Mesh;
	TObjectPtr<class ASkeletalMeshActor> Character3Mesh;
	TObjectPtr<class ASkeletalMeshActor> Character4Mesh;

	UPROPERTY(meta = (BindWidget))
	class UButton* Character1Button;

	UPROPERTY(meta = (BindWidget))
	class UButton* Character2Button;

	UPROPERTY(meta = (BindWidget))
	class UButton* Character3Button;

	UPROPERTY(meta = (BindWidget))
	class UButton* Character4Button;

	UPROPERTY(meta = (BindWidget))
	class UButton* MatchingButton;

	UFUNCTION()
	void Matching();
	UFUNCTION()
	void Character1ButtonPressed();
	UFUNCTION()
	void Character2ButtonPressed();
	UFUNCTION()
	void Character3ButtonPressed();
	UFUNCTION()
	void Character4ButtonPressed();

	UFUNCTION()
	void EndHovered();
	UFUNCTION()
	void Button1Hovered();
	UFUNCTION()
	void Button2Hovered();
	UFUNCTION()
	void Button3Hovered();
	UFUNCTION()
	void Button4Hovered();
	class UBOGameInstance* inst;
private:
	ACharacterController* MyCharacterController;
	bool bClicked;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> ClickSound;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> HoverSound;

};
