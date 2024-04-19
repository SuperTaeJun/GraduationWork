// Fill out your copyright notice in the Description page of Project Settings.


#include "GameProp/EscapePlace.h"
#include "Components/BoxComponent.h"
#include "Character/CharacterBase.h"
// Sets default values
AEscapePlace::AEscapePlace()
{
	PrimaryActorTick.bCanEverTick = true;

	AircraftMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AircraftMesh"));
	SetRootComponent(AircraftMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AEscapePlace::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEscapePlace::OnBoxOverlap);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &AEscapePlace::OnBoxEndOverlap);
}

// Called every frame
void AEscapePlace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEscapePlace::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacterBase* InPlaceCh = Cast<ACharacterBase>(OtherActor);
	if (InPlaceCh && InPlaceCh->GetEscapeToolNum()>=10)
	{
		InPlaceCh->SetbCanEscape(true);
	}
}

void AEscapePlace::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACharacterBase* InPlaceCh = Cast<ACharacterBase>(OtherActor);
	if (InPlaceCh)
	{
		InPlaceCh->SetbCanEscape(false);
	}
}

