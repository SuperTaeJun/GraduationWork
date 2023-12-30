// Fill out your copyright notice in the Description page of Project Settings.


#include "Animatiom/BOAnimInstance.h"
#include "Character/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
void UBOAnimInstance::NativeInitializeAnimation()
{
	BaseCharacter = Cast<ACharacterBase>(GetOwningActor());
	if (BaseCharacter)
	{
		Movement = BaseCharacter->GetCharacterMovement();
	}
}

void UBOAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (Movement)
	{
		FVector Velocity = Movement->Velocity;
		Speed = Velocity.Size2D();

		bIsFalling = Movement->IsFalling();
		bIsAccelerating = BaseCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	}
}
