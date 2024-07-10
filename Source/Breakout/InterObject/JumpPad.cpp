// Fill out your copyright notice in the Description page of Project Settings.


#include "InterObject/JumpPad.h"
#include "Components/BoxComponent.h"
#include "Character/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include"Animatiom/BOAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
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
		FVector Impulse = (InCh->GetActorForwardVector() + FVector(0.f, 0.f, 1.f)) * FVector(10.f, 10.f, JumpPowerZ);
		InCh->GetCharacterMovement()->AddImpulse(Impulse, true);
		//점프 falling상태로 변경해줘야함
		if (InCh->GetMainController())
			InCh->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
	if (ImpactNiagara)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagara, GetActorLocation());
	if (JumpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation());
	}
}


