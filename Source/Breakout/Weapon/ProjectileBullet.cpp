// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileBullet.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include"Character/CharacterBase.h"
#include "GameProp/BulletHoleWall.h"
#include "Game/BOGameInstance.h"
#include "ClientSocket.h"
#include "TimerManager.h"
// Sets default values
AProjectileBullet::AProjectileBullet()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	BeamNiagaraMesh = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileMesh"));
	BeamNiagaraMesh->SetupAttachment(RootComponent);
	BeamNiagaraMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;

	ImpactNiagara = ConstructorHelpers::FObjectFinder<UNiagaraSystem>(TEXT("/Script/Niagara.NiagaraSystem'/Game/Niagara/Weapon/RifleAndShotgun/NS_Impact.NS_Impact'")).Object;

	bHit = false;
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(DistanceTimerHandle, this,&AProjectileBullet::DistanceTimer, Distance);

	CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileBullet::OnHit);

	BeamNiagaraMesh->SetVectorParameter(FName("Start"), GetActorLocation());
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NomalImpulse, const FHitResult& Hit)
{
	bHit = true;

	APawn* FiringPawn = GetInstigator();
	AController* FiringController = FiringPawn->GetController();
	if (FiringPawn)
	{
		ACharacterBase* DamagedCharacter=Cast<ACharacterBase>(OtherActor);
		ABulletHoleWall* DamagedWall = Cast<ABulletHoleWall>(OtherActor);
		if(DamagedCharacter)
			UGameplayStatics::ApplyDamage(DamagedCharacter,Damage,FiringController,FiringPawn,UDamageType::StaticClass());
		else if (DamagedWall)
		{
			UE_LOG(LogTemp, Warning, TEXT("DamagedWall"));
			DamagedWall->SetBulletHole(Hit.ImpactPoint);
		}
	}
	Destroy();
}

void AProjectileBullet::Destroyed()
{
	if (bHit)
	{
		if (ImpactNiagara)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagara, GetActorLocation());
		}
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
		}
	}
}

void AProjectileBullet::DistanceTimer()
{
	Destroy();
}

// Called every frame
void AProjectileBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector BeamEnd = GetActorLocation();
	if (BeamNiagaraMesh)
	{
		BeamNiagaraMesh->SetVectorParameter(FName("End"), BeamEnd);
	}
}

