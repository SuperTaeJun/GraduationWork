// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character4.h"
#include "Skill/SkillComponent.h"

void ACharacter4::BeginPlay()
{
	Super::BeginPlay();

	SkillComp->SetSelectedSkill(ESelectedSkill::E_Skill4);
}

void ACharacter4::Skill_S(const FInputActionValue& Value)
{
	if (SkillComp->Toggle % 2 == 1)
	{
		SkillComp->SaveCurLocation();
	}
	else
	{
		SkillComp->SetLocation();
	}
}

void ACharacter4::Skill_E(const FInputActionValue& Value)
{
}
