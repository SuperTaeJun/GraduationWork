// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Character2.h"
#include "Skill/SkillComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void ACharacter2::BeginPlay()
{
	Super::BeginPlay();


	SkillComp->SetSelectedSkill(ESelectedSkill::E_Skill2);
}

void ACharacter2::Skill_S(const FInputActionValue& Value)
{
	if (SkillComp->GetDashPoint() > 0 && !Movement->IsFalling())
	{
		SkillComp->SetIsDash(true);
		SkillComp->DashStart();
	}
}

void ACharacter2::Skill_E(const FInputActionValue& Value)
{
}
