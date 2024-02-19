// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character1.h"
#include "Skill/SkillComponent.h"
void ACharacter1::BeginPlay()
{
	Super::BeginPlay();

	SkillComp->SetSelectedSkill(ESelectedSkill::E_Skill1);
}
void ACharacter1::Skill_S(const FInputActionValue& Value)
{
	SkillComp->SetIsReverse(true);
}

void ACharacter1::Skill_E(const FInputActionValue& Value)
{
	SkillComp->SetIsReverse(false);
}
