// Fill out your copyright notice in the Description page of Project Settings.


#include "InterObject/InterActorBase.h"
#include "Components/BoxComponent.h"
#include "Character/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
// Sets default values
AInterActorBase::AInterActorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ActorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ActorMesh"));
	SetRootComponent(ActorMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AInterActorBase::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AInterActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

