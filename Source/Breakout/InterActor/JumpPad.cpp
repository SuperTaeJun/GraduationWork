// Fill out your copyright notice in the Description page of Project Settings.


#include "InterActor/JumpPad.h"
#include "Components/BoxComponent.h"
#include "Character/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void AJumpPad::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AJumpPad::OnBoxOverlap);
}

void AJumpPad::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacterBase* InCh = Cast<ACharacterBase>(OtherActor);
	if (InCh)
	{
		FVector Impulse = (InCh->GetActorForwardVector() + FVector(0.f, 0.f, 1.f)) * FVector(10.f, 10.f, 1300.f);
		InCh->GetCharacterMovement()->AddImpulse(Impulse, true);
		UE_LOG(LogTemp, Warning, TEXT("JUMP"));
	}
	if(ImpactNiagara)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagara, GetActorLocation());
}

