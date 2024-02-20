// Fill out your copyright notice in the Description page of Project Settings.


#include "FX/ReplayFX.h"

// Sets default values
AReplayFX::AReplayFX()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AReplayFX::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AReplayFX::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

