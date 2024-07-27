
#include "Weapon/ProjectileBase.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameProp/BulletHoleWall.h"
#include "Weapon/ProjectileBoobyTrap.h"
#include "Character/CharacterBase.h"
AProjectileBase::AProjectileBase()
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
	
	//BeamNiagaraMesh->setniagara
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetStaticMesh(
		ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("/Script/Engine.StaticMesh'/DatasmithContent/Meshes/sphere.sphere'")).Object);
	ProjectileMesh->SetWorldScale3D(FVector(30.f, 30.f, 30.f));
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;

	//ImpactNiagara = ConstructorHelpers::FObjectFinder<UNiagaraSystem>(TEXT("/Script/Niagara.NiagaraSystem'/Game/Niagara/Weapon/Lancher/NS_Explosion.NS_Explosion'")).Object;

	Damage = 50.f;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	//SetAllowHitEventTimer();
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NomalImpulse, const FHitResult& Hit)
{
	//UE_LOG(LogTemp, Log, TEXT("ONHIT1"));
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		//UE_LOG(LogTemp, Log, TEXT("ONHIT2"));
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // World context object
				Damage, // BaseDamage
				30.f, // MinimumDamage
				GetActorLocation(), // Origin
				100.f, // DamageInnerRadius
				300.f, // DamageOuterRadius
				1.f, // DamageFalloff
				UDamageType::StaticClass(), // DamageTypeClass
				TArray<AActor*>(), // IgnoreActors
				this, // DamageCauser
				FiringController // InstigatorController
			);

		}
		ABulletHoleWall* DamagedWall = Cast<ABulletHoleWall>(OtherActor);
		AProjectileBoobyTrap* DamagedTrap = Cast<AProjectileBoobyTrap>(OtherActor);
		if (DamagedWall)
		{
			UGameplayStatics::ApplyDamage(DamagedWall, 99999.f, FiringController, FiringPawn, UDamageType::StaticClass());
		}
		else if (DamagedTrap)
		{
			UGameplayStatics::ApplyDamage(DamagedWall, 99999.f, FiringController, FiringPawn, UDamageType::StaticClass());
		}


	//	DrawDebugSphere(GetWorld(), GetActorLocation(), 100.f, 20, FColor::Black, false, 10, 0, 1);
	//	DrawDebugSphere(GetWorld(), GetActorLocation(), 300.f, 20, FColor::Purple, false, 10, 0, 1);
		Destroy();
	}
}

void AProjectileBase::AllowHitEvent()
{
	CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
}

void AProjectileBase::SetAllowHitEventTimer()
{
	GetWorldTimerManager().SetTimer(HitTimer, this, &AProjectileBase::AllowHitEvent, 0.1);
}

void AProjectileBase::Destroyed()
{
	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactNiagara, GetActorTransform().GetLocation());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

