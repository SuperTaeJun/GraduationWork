// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/RespawnSelect.h"
#include "Components/Button.h"

void URespawnSelect::NativeConstruct()
{
	Super::NativeConstruct();

	Respawn1->OnClicked.AddDynamic(this, &URespawnSelect::Respawn1Pressed);
	Respawn2->OnClicked.AddDynamic(this, &URespawnSelect::Respawn2Pressed);
	Respawn3->OnClicked.AddDynamic(this, &URespawnSelect::Respawn3Pressed);
	Respawn4->OnClicked.AddDynamic(this, &URespawnSelect::Respawn4Pressed);
}

void URespawnSelect::Respawn1Pressed()
{
	FName TagName = FName(TEXT("PlayerStart1"));
}

void URespawnSelect::Respawn2Pressed()
{
	FName TagName = FName(TEXT("PlayerStart2"));
}

void URespawnSelect::Respawn3Pressed()
{
	FName TagName = FName(TEXT("PlayerStart3"));
}

void URespawnSelect::Respawn4Pressed()
{
	FName TagName = FName(TEXT("PlayerStart4"));
}
