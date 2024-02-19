// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character3.h"
#include "Skill/SkillComponent.h"

void ACharacter3::BeginPlay()
{
	Super::BeginPlay();

	SkillComp->SetSelectedSkill(ESelectedSkill::E_Skill3);
}

void ACharacter3::Skill_S(const FInputActionValue& Value)
{
	SkillComp->SetIsGhost(true);
}

void ACharacter3::Skill_E(const FInputActionValue& Value)
{
	SkillComp->SetIsGhost(false);
	SkillComp->SetIsCharageTime(false);
}
