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
		TurningType = BaseCharacter->GetTurningType();
		//Yaw������ ��
		FRotator AimRotation = BaseCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BaseCharacter->GetVelocity());
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
		DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
		YawOffset = DeltaRotation.Yaw;

		//Lean������ ��
		CharacterRotationLastFrame = CharacterRotation;
		CharacterRotation = BaseCharacter->GetActorRotation();
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
		const float Target = Delta.Yaw / DeltaSeconds;
		const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
		Lean = FMath::Clamp(Interp, -90.f, 90.f);

		AO_Yaw = BaseCharacter->GetAO_Yaw();
		AO_Pitch = BaseCharacter->GetAO_Pitch();

	}
}
