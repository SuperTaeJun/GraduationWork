// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWall.h"
#include "GameProp/Wall.h"
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


	if (WallClass && InstigatorPawn)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = GetOwner();
		SpawnParameters.Instigator = InstigatorPawn;


		FRotator Rotation = InstigatorPawn->GetActorForwardVector().Rotation();
		UWorld* World = GetWorld();
		if (World)
		{
			FVector ConvertLoc = Hit.Location;
			ConvertLoc.X += 200.f;
			World->SpawnActor<ABulletHoleWall>(WallClass, ConvertLoc, Rotation, SpawnParameters);
		}
	}
	
	Destroy();

}
