// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/RocketLauncher.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/ProjectileBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
void ARocketLauncher::Fire(const FVector& HitTarget)
{
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket && InstigatorPawn)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		StartPos = SocketTransform.GetLocation();
		CurWeaponRot = TargetRotation;
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = GetOwner();
			SpawnParameters.Instigator = InstigatorPawn;
			SpawnParm = SpawnParameters;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectileBase>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParameters);
			}
			if (ImpactNiagara)
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					World,
					ImpactNiagara,
					SocketTransform.GetLocation()
				);

			}
		}

	}
}

void ARocketLauncher::SpawnProjectile()
{
	GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, FVector(-2000,100,100), FRotator::ZeroRotator, SpawnParm);

}
