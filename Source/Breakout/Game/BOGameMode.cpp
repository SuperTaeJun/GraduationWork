// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BOGameMode.h"
#include "Character/CharacterBase.h"
ABOGameMode::ABOGameMode()
{

	static ConstructorHelpers::FClassFinder<APawn> BaseCharacterRef(TEXT("/Game/BP/Character/BP_CharacterBase.BP_CharacterBase_C"));

	DefaultPawnClass = BaseCharacterRef.Class;
}
