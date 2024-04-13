// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ShotGun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Character/CharacterBase.h"
#include "ClientSocket.h"
#include "DrawDebugHelpers.h"
#include "Game/BOGameInstance.h"
#include "GameProp/BulletHoleWall.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Weapon/ProjectileBullet.h"

AShotGun::AShotGun()
{
}

void AShotGun::Fire(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FVector ToTarget = bUseScatter ? TraceEndWithScatter(Start, HitTarget) : Start + (HitTarget - Start) * 1.25f;
			ToTarget = ToTarget - SocketTransform.GetLocation();
			FRotator ToTargetRot = ToTarget.Rotation();

			if (ProjectileBulletClass && OwnerPawn)
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.Owner = GetOwner();
				SpawnParameters.Instigator = OwnerPawn;
				UWorld* World = GetWorld();
				if (World)
				{
					World->SpawnActor<AProjectileBullet>(ProjectileBulletClass, SocketTransform.GetLocation(), ToTargetRot, SpawnParameters);
					//서버 총알생성
				}
			}

		}
	}
	//		/*	FVector End = TraceEndWithScatter(Start, HitTarget);*/
	//		FHitResult FireHit;
	//		FVector EndBeamLoc;
	//		ShotGunTraceHit(Start, HitTarget, FireHit, EndBeamLoc);
	//		//WeaponTraceHit(Start, HitTarget, FireHit);
	//		ServerBeamStart.Add(Start);
	//		ServerBeamEnd.Add(EndBeamLoc);
	//		ACharacterBase* CharacterBase = Cast<ACharacterBase>(FireHit.GetActor());
	//		ABulletHoleWall* DamagedWall = Cast<ABulletHoleWall>(FireHit.GetActor());
	//		if (CharacterBase && HasAuthority() && InstigatorController)
	//		{
	//			if (HitMap.Contains(CharacterBase))
	//			{
	//				HitMap[CharacterBase]++;
	//			}
	//			else
	//			{
	//				HitMap.Emplace(CharacterBase, 1);
	//			}
	//		}
	//		else if (DamagedWall)
	//		{
	//			DamagedWall->SetBulletHole(FireHit.ImpactPoint);
	//		}

	//		if (ImpactNiagara && FireHit.bBlockingHit)
	//		{
	//			UNiagaraFunctionLibrary::SpawnSystemAtLocation
	//			(
	//				GetWorld(),
	//				ImpactNiagara,
	//				FireHit.ImpactPoint,
	//				FireHit.ImpactNormal.Rotation()
	//			);
	//			ServerImpactLoc.Add(FireHit.ImpactPoint);
	//			ServerImpactRot.Add(FireHit.ImpactNormal.Rotation());
	//		}

	//	}
	//	for (int i = 0; i < 10; ++i)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("ServerBeamEnd : %s"), *ServerBeamEnd[i].ToString());
	//	}
	//	Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_ShotGun_packet(Cast<ACharacterBase>(GetOwner())->_SessionId, ServerBeamStart, ServerBeamEnd, ServerBeamStart.Num());

	//	if (ServerImpactRot.Num() > 0)
	//	{
	//		// 임펙트 나이아가라 -> 배열의 크기는 항상5가아님 총알이 맞는횟수만큼

	//	}
	//	//서버 데미지 적용 변수
	//	TArray<ACharacterBase*> DamagedCh;
	//	DamagedCh.SetNum(3);
	//	DamagedCh[0] = nullptr;
	//	DamagedCh[1] = nullptr;
	//	DamagedCh[2] = nullptr;
	//	TArray<int32> DamageNum;
	//	DamageNum.SetNum(3);
	//	DamageNum[0] = 0;
	//	DamageNum[1] = 0;
	//	DamageNum[2] = 0;
	//	int i = 0;
	//	for (auto HitPair : HitMap)
	//	{
	//		++i;
	//		if (HitPair.Key && HasAuthority() && InstigatorController)
	//		{
	//			DamagedCh[i] = HitPair.Key;
	//			DamageNum[i] = HitPair.Value;
	//			UGameplayStatics::ApplyDamage
	//			(
	//				HitPair.Key,
	//				Damage * HitPair.Value,
	//				InstigatorController,
	//				this,
	//				UDamageType::StaticClass()
	//			);
	//		}
	//	}
	//	// 여기 데미지
	//	// 모두 데미지받았을때
	//	if (DamagedCh[0] && DamagedCh[1] && DamagedCh[2])
	//	{
	//		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_ShotGun_damaged_packet(DamagedCh[0]->_SessionId, DamagedCh[1]->_SessionId, DamagedCh[2]->_SessionId
	//			, DamageNum[0]*Damage, DamageNum[1] * Damage, DamageNum[2] * Damage);
	//	}
	//	else if(DamagedCh[0] && DamagedCh[1] && !DamagedCh[2])
	//	{
	//		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_ShotGun_damaged_packet(DamagedCh[0]->_SessionId, DamagedCh[1]->_SessionId, -1
	//			, DamageNum[0] * Damage, DamageNum[1] * Damage, 0);
	//	}
	//	else if (DamagedCh[0] && !DamagedCh[1] && DamagedCh[2])
	//	{
	//		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_ShotGun_damaged_packet(DamagedCh[0]->_SessionId, -1, DamagedCh[2]->_SessionId
	//			, DamageNum[0] * Damage, 0, DamageNum[2] * Damage);
	//	}
	//	else if (!DamagedCh[0] && DamagedCh[1] && DamagedCh[2])
	//	{
	//		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_ShotGun_damaged_packet(-1, DamagedCh[1]->_SessionId, DamagedCh[2]->_SessionId
	//			,0, DamageNum[1] * Damage, DamageNum[2] * Damage);
	//	}
	//	else if (!DamagedCh[0] && !DamagedCh[1] && DamagedCh[2])
	//	{
	//		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_ShotGun_damaged_packet(-1, -1, DamagedCh[2]->_SessionId
	//			, 0, 0, DamageNum[2] * Damage);
	//	}
	//	else if (!DamagedCh[0] && DamagedCh[1] && !DamagedCh[2])
	//	{
	//		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_ShotGun_damaged_packet(-1, DamagedCh[1]->_SessionId, -1
	//			, 0, DamageNum[1] * Damage, 0);
	//	}
	//	else if (DamagedCh[0] && !DamagedCh[1] && !DamagedCh[2])
	//	{
	//		Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_ShotGun_damaged_packet(DamagedCh[0]->_SessionId, -1, -1
	//			, DamageNum[0] * Damage, 0, 0);
	//	}
	//	/*Cast<UBOGameInstance>(GetGameInstance())->m_Socket->Send_ShotGun_damaged_packet(DamagedCh[0]->_SessionId);*/
	//}
}
