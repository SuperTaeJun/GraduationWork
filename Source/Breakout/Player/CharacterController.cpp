// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CharacterController.h"
void ACharacterController::BeginPlay()
{
	FInputModeGameOnly GameOnlyInput;
	SetInputMode(GameOnlyInput);
}
