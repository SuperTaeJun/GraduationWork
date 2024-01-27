// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Grenade.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGrenade::AGrenade()
{
	ProjectileMovementComponent->bShouldBounce = true;

}
