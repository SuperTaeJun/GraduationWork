// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/Skill4Actor.h"
#include "TimerManager.h"
void ASkill4Actor::BeginPlay()
{
	Super::BeginPlay();
}

void ASkill4Actor::Tick(float Delta)
{
	if (bTimerStart)
	{
		GetWorld()->GetTimerManager().SetTimer(TelpoTimer, this, &ASkill4Actor::Dead, 0.5f, false);
		bTimerStart = false;
	}
}

void ASkill4Actor::Dead()
{
	Destroy();
}
