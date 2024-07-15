// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWall.h"
#include "GameProp/Wall.h"
#include "Kismet/GameplayStatics.h"
#include "GameProp/BulletHoleWall.h"
AProjectileWall::AProjectileWall()
{
	ImpactNiagara = nullptr;

}
void AProjectileWall::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileWall::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NomalImpulse, const FHitResult& Hit)
{
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());


	if (WallClass && InstigatorPawn!=OtherActor)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.Instigator = InstigatorPawn;

		FRotator Rotation = InstigatorPawn->GetActorForwardVector().Rotation();
		UWorld* World = GetWorld();
		if (World)
		{
			FVector ConvertLoc = Hit.Location;
			ConvertLoc.Z += 100.f;
			TArray<AActor*> Walls;
			UGameplayStatics::GetAllActorsOfClass(World, WallClass, Walls);

			for (auto wall:Walls)
			{
				ABulletHoleWall* BulletWall = Cast<ABulletHoleWall>(wall);
				if (BulletWall && BulletWall->bUsing)
				{
					BulletWall->SetActorLocationAndRotation(ConvertLoc, Rotation);
					BulletWall->bUsing = false;
					Destroy();
					return;
				}
				else
				{
					Destroy();
				}
			}

			//World->SpawnActor<ABulletHoleWall>(WallClass, ConvertLoc, Rotation, SpawnParameters);
		}


	}

}
