// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SelectWeaponUi.h"
#include "Components/Button.h"
#include "Weapon/WeaponBase.h"
#include "GameFramework/PlayerController.h"
#include "Character/CharacterBase.h"
#include "Player/CharacterController.h"
#include "MainHUD.h"
void USelectWeaponUi::NativeConstruct()
{
	Super::NativeConstruct();

	RifleButton->OnClicked.AddDynamic(this, &USelectWeaponUi::RifleButtonPressed);
	ShotgunButton->OnClicked.AddDynamic(this, &USelectWeaponUi::ShotgunButtonPressed);
	LancherButton->OnClicked.AddDynamic(this, &USelectWeaponUi::LancherButtonPressed);

}

void USelectWeaponUi::RifleButtonPressed()
{
	UE_LOG(LogTemp, Log, TEXT("RifleButtonPressed"));
	ACharacterBase* Character=Cast<ACharacterBase>(GetOwningPlayerPawn());
	ACharacterController* Controller = Cast<ACharacterController>(Character->Controller);
	AMainHUD* MainHUD = Cast<AMainHUD>(Controller->GetHUD());

	FInputModeGameOnly GameOnlyInput;
	Controller->SetInputMode(GameOnlyInput);
	MainHUD->RemoveSelectWeapon();
	Controller->bShowMouseCursor = false;
	Controller->bEnableMouseOverEvents = false;

	FName RifleSocketName = FName("RifleSocket");
	Character->SetWeapon(Rifle, RifleSocketName);
	Controller->SetHUDAmmo(30);
}

void USelectWeaponUi::ShotgunButtonPressed()
{
	UE_LOG(LogTemp, Log, TEXT("ShotgunButtonPressed"));
	ACharacterBase* Character = Cast<ACharacterBase>(GetOwningPlayerPawn());
	ACharacterController* Controller = Cast<ACharacterController>(Character->Controller);
	AMainHUD* MainHUD = Cast<AMainHUD>(Controller->GetHUD());

	FInputModeGameOnly GameOnlyInput;
	Controller->SetInputMode(GameOnlyInput);
	MainHUD->RemoveSelectWeapon();
	Controller->bShowMouseCursor = false;
	Controller->bEnableMouseOverEvents = false;

	FName ShotguSocketName = FName("ShotgunSocket");

	Character->SetWeapon(ShotGun,ShotguSocketName);
	Controller->SetHUDAmmo(5);
}

void USelectWeaponUi::LancherButtonPressed()
{
	UE_LOG(LogTemp, Log, TEXT("LancherButtonPressed"));
	ACharacterBase* Character = Cast<ACharacterBase>(GetOwningPlayerPawn());
	ACharacterController* Controller = Cast<ACharacterController>(Character->Controller);
	AMainHUD* MainHUD = Cast<AMainHUD>(Controller->GetHUD());

	FInputModeGameOnly GameOnlyInput;
	Controller->SetInputMode(GameOnlyInput);
	MainHUD->RemoveSelectWeapon();
	Controller->bShowMouseCursor = false;
	Controller->bEnableMouseOverEvents = false;

	FName LancherSocketName = FName("LancherSocket");

	Character->SetWeapon(Lancher, LancherSocketName);
	Controller->SetHUDAmmo(2);
}
