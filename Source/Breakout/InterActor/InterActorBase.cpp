// Fill out your copyright notice in the Description page of Project Settings.


#include "InterActor/InterActorBase.h"
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
	
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AInterActorBase::OnBoxOverlap);
}

// Called every frame
void AInterActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInterActorBase::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacterBase* InCh = Cast<ACharacterBase>(OtherActor);
	if (InCh)
	{
		FVector Impulse = (InCh->GetActorForwardVector() + FVector(0.f, 0.f, 1.f)) * FVector(10.f, 10.f, 1300.f);
		InCh->GetCharacterMovement()->AddImpulse(Impulse,true);
		UE_LOG(LogTemp, Warning, TEXT("JUMP"));
	}
}

