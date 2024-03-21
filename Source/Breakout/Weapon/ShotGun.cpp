// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ShotGun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Character/CharacterBase.h"
#include "DrawDebugHelpers.h"
#include "GameProp/BulletHoleWall.h"
void AShotGun::Fire(const FVector& HitTarget)
{
	//AWeaponBase::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<ACharacterBase*, uint32> HitMap;
		TMap<ABulletHoleWall*, uint32> HitWall;
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			/*	FVector End = TraceEndWithScatter(Start, HitTarget);*/
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ACharacterBase* CharacterBase = Cast<ACharacterBase>(FireHit.GetActor());
			ABulletHoleWall* DamagedWall = Cast<ABulletHoleWall>(FireHit.GetActor());
			if (CharacterBase && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(CharacterBase))
				{
					HitMap[CharacterBase]++;
				}
				else
				{
					HitMap.Emplace(CharacterBase, 1);
				}
			}
			else if (DamagedWall)
			{
				DamagedWall->SetBulletHole(FireHit.ImpactPoint);
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}

		}
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}
